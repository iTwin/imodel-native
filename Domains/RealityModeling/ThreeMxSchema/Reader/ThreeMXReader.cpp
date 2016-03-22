/*-------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/Reader/ThreeMXReader.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"

#include "openctm/openctm.h"

#define LOG_ERROR(...)

template <typename T> T getJsonValue(JsonValueCR pt);
template<> double getJsonValue(JsonValueCR pt) {return pt.asDouble();}
template<> size_t getJsonValue(JsonValueCR pt) {return pt.asUInt();}
template<> Utf8String getJsonValue(JsonValueCR pt) {return pt.asCString();}

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void readVector(JsonValueCR pt, bvector<T>& v)
    {
    v.clear();
    for (Json::ArrayIndex i = 0; i < pt.size(); ++i)
        {
        v.push_back(getJsonValue<T>(pt[i]));
        }
    }

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

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool readNodeInfo(JsonValueCR pt, S3NodeInfo& nodeInfo, Utf8String& nodeName, bvector<Utf8String>& nodeResources)
    {
    nodeName = pt.get("id", "").asCString();

    if (nodeName.empty())
        {
        LOG_ERROR("No node id");
        return false;
        }

    bvector<double> bbMin;
    bvector<double> bbMax;
    bvector<double> sphereCenter;
    sphereCenter.resize(3);

    double sphereDiameter;

    if (!readVectorEntry(pt, "bbMin", bbMin)) 
        return false;

    if (bbMin.size() != 3)
        {
        LOG_ERROR("Malformed bbMin");
        return false;
        }

    if (!readVectorEntry(pt, "bbMax", bbMax)) 
        return false;

    if (bbMax.size() != 3)
        {
        LOG_ERROR("Malformed bbMax");
        return false;
        }

    sphereDiameter = 0;
    for (int i = 0; i < 3; i++) 
        {
        sphereCenter[i] = (bbMin[i] + bbMax[i])*0.5;
        sphereDiameter += (bbMax[i] - bbMin[i])*(bbMax[i] - bbMin[i]);
        }
    nodeInfo.m_radius = 0.5 * sqrt(sphereDiameter);
    nodeInfo.m_center = DPoint3d::FromArray(&sphereCenter.front());

    Json::Value val = pt["maxScreenDiameter"];
    if (val.empty())
        {
        LOG_ERROR("Cannot find \"maxScreenDiameter\" entry");
        return false;
        }
    nodeInfo.m_dMax = val.asDouble();
    if (!readVectorEntry(pt, "resources", nodeResources))
        {
        LOG_ERROR("Cannot find \"resources\" entry");
        return false;
        }

    if (!readVectorEntry(pt, "children", nodeInfo.m_children))
        return false;

    return true;
    }

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
        return false;

    memcpy (buf, start, size);
    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t ctmReadFunc(void* buf, uint32_t count, void* userData)
    {
    return readBytes(*(MxStreamBuffer*)userData, buf, count) ? count : 0;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BaseMeshNode::Read3MXB(MxStreamBuffer& in)
    {
    _Clear();
    bmap<Utf8String, int>  textureIds;
    bmap<Utf8String, int>  nodeIds;
    bmap<Utf8String, Utf8String>  geometryNodeCorrespondence;
    int     textureCount = 0, nodeCount = 0;

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
        return (BentleyStatus)ERROR;
        }

    int version = pt.get("version", 0).asInt();

    if (version != 1)
        {
        LOG_ERROR("Unsupported version");
        return ERROR;
        }

    Json::Value entry = pt["nodes"];
    if (!entry.empty())
        {
        for (Json::ArrayIndex i = 0; i < entry.size(); i++)
            {
            S3NodeInfo nodeInfo;
            Utf8String nodeName;
            bvector<Utf8String> nodeResources;

            if (!readNodeInfo(entry[i], nodeInfo, nodeName, nodeResources))
                return (BentleyStatus)ERROR;

            nodeIds[nodeName] = nodeCount++;
            for (size_t i = 0; i < nodeResources.size(); i++){
                geometryNodeCorrespondence[nodeResources[i]] = nodeName;
            }
        _PushNode(nodeInfo);
        }

    Utf8String resourceType;
    Utf8String resourceFormat;
    Utf8String resourceName;
    uint32_t resourceSize;
    uint32_t offset = (uint32_t) ThreeMX::GetMagicNumber().size() + 4 + infoSize;

    entry = pt["resources"];

    if (!entry.empty())
        for (Json::ArrayIndex i = 0; i < entry.size(); i++)
            {
            JsonValueCR childPt = entry[i];
            resourceType = childPt.get("type", "").asCString();
            resourceFormat = childPt.get("format", "").asCString();
            resourceName = childPt.get("id", "").asCString();
            resourceSize = (size_t)childPt.get("size", 0).asUInt();

            if (resourceType == "textureBuffer" && resourceFormat == "jpg"
                && !resourceName.empty() && resourceSize > 0)
                {
                in.SetPos(offset);
                ByteCP buffer=in.GetCurrent();
                if (!in.Advance(resourceSize))
                    {
                    LOG_ERROR("Cannot read child data");
                    _Clear();
                    return ERROR;
                    }

                _PushJpegTexture(buffer, resourceSize);
                textureIds[resourceName] = textureCount++;
                }
            else if (resourceType == "geometryBuffer" && resourceFormat == "ctm"
                    && !resourceName.empty() && resourceSize > 0)
                {
                if (geometryNodeCorrespondence.find(resourceName) == geometryNodeCorrespondence.end())
                    {
                    LOG_ERROR("Geometry is not referenced by any node");
                    _Clear();
                    return ERROR;
                    }
                Utf8String nodeName = geometryNodeCorrespondence[resourceName];
                if (nodeIds.find(nodeName) == nodeIds.end())
                    {
                    LOG_ERROR("Node name is unknown");
                    _Clear();
                    return ERROR;
                    }
                int nodeId = nodeIds[nodeName];

                CTMcontext context;
                context = ctmNewContext(CTM_IMPORT);
                if (ctmGetError(context) != CTM_NONE)
                    {
                    LOG_ERROR(Utf8String("CTM context error: ") + ctmErrorString(ctmGetError(context)));
                    _Clear();
                    return ERROR;
                    }

                in.SetPos(offset);
                ctmLoadCustom(context, ctmReadFunc, &in);
                if (ctmGetError(context) != CTM_NONE)
                    {
                    LOG_ERROR(Utf8String("CTM read error: ") + ctmErrorString(ctmGetError(context)));
                    _Clear();
                    return ERROR;
                    }

                unsigned int nbVertices = ctmGetInteger(context, CTM_VERTEX_COUNT);
                float* positions = (float*)ctmGetFloatArray(context, CTM_VERTICES);

                float* normals = (ctmGetInteger(context, CTM_HAS_NORMALS) == CTM_TRUE) ? (float*)ctmGetFloatArray(context, CTM_NORMALS) : NULL;

                unsigned int nbTriangles = ctmGetInteger(context, CTM_TRIANGLE_COUNT);
                int* indices = (int*)ctmGetIntegerArray(context, CTM_INDICES);

                //Texture
                unsigned int nbTexCoordsArrays = ctmGetInteger(context, CTM_UV_MAP_COUNT);
                float* textureCoordinates = NULL;
                int textureId = -1;
                if (nbTexCoordsArrays == 1)
                    {
                    Utf8String texName = childPt.get("texture", Json::Value("")).asCString();
                    if (!texName.empty())
                        {
                        textureCoordinates = (float*)ctmGetFloatArray(context, CTM_UV_MAP_1);
                        if (textureIds.find(texName) == textureIds.end())
                            {
                            LOG_ERROR(Utf8String("Bad texture name ") + texName);
                            ctmFreeContext(context);
                            _Clear();
                            return ERROR;
                            }
                        textureId = textureIds[texName];
                        }
                    }
                _AddGeometry(nodeId, nbVertices, positions, normals, nbTriangles, indices, textureCoordinates, textureId);
                ctmFreeContext(context);
                }//end CTM
            else 
                {
                LOG_ERROR("Bad children definition: resourceType = " + resourceType + "; resourceFormat = " + resourceFormat + "; resourceName = " + resourceName);
                _Clear();
                return (BentleyStatus)ERROR;
                }
            offset += resourceSize;
            }
        }
    return (BentleyStatus)SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BaseMeshNode::Read3MXB(BeFileNameCR filename)
    {
    _Clear();

    _SetDirectory(BeFileName(BeFileName::DevAndDir, filename.c_str()));
    BeFile file;
    if (BeFileStatus::Success != file.Open(filename.c_str(), BeFileAccess::Read))
        {
        LOG_ERROR("Cannot open file");
        return ERROR;
        }

    MxStreamBuffer buf;
    file.ReadEntireFile(buf);
    file.Close();

    return Read3MXB(buf);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus S3SceneInfo::Read3MX(MxStreamBuffer& buffer) 
    {
    Json::Value pt;
    Json::Reader reader;
    Utf8CP buffStr = (Utf8CP) buffer.GetData();
    if (!reader.parse(buffStr, buffStr+buffer.GetSize(), pt))
        {
        LOG_ERROR("Cannot parse info");
        return ERROR;
        }

    int version = pt.get(ThreeMX::GetSceneVersionTag().c_str(), 0).asInt();
    if (version != 1)
        {
        LOG_ERROR(Utf8String("Unsupported version of ") + ThreeMX::GetShortName());
        return ERROR;
        }

    m_sceneName = pt.get("name", "").asCString();

    Json::Value entry = pt["sceneOptions"];
    if (!entry.empty()){
        for (Json::ArrayIndex i = 0; i < entry.size(); i++)
            {
            m_navigationMode = entry[i].get("navigationMode", "").asCString();
            }
        }

    /*Json::Value */entry = pt["layers"];
    Utf8String meshType;
    Utf8String meshPath;
    if (entry.empty())
        {
        LOG_ERROR("Cannot find \"layers\" tag");
        return ERROR;
        }

    for (Json::ArrayIndex i = 0; i < entry.size(); i++)
        {
        JsonValueCR childPt = entry[i];
        meshType = childPt.get("type", "").asCString();
        if (meshType == "meshPyramid")
            {
            meshPath = childPt.get("root", "").asCString();
            if (meshPath.empty())
                {
                LOG_ERROR("Could not find the mesh path");
                return ERROR;
                }
            m_meshChildren.push_back(meshPath);
            m_SRS = childPt.get("SRS", "").asCString();
            if (!m_SRS.empty())
                {
                if (!readVectorEntry(childPt, "SRSOrigin", m_SRSOrigin))
                    {
                    LOG_ERROR("Could not find the SRS origin");
                    return ERROR;
                    }
                if (m_SRSOrigin.size() != 3)
                    {
                    LOG_ERROR("Malformed SRS origin");
                    return ERROR;
                    }
                }
            }
        }
    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus S3SceneInfo::Read3MX(BeFileNameCR filename)
    {
    BeFile file;
    if (BeFileStatus::Success != file.Open(filename.c_str(), BeFileAccess::Read))
        {
        LOG_ERROR("Cannot open file");
        return ERROR;
        }

    MxStreamBuffer buf;
    file.ReadEntireFile(buf);
    file.Close();

    return Read3MX(buf);
    }
