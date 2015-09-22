/*-------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/Reader/ThreeMXReader.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "..\ThreeMxSchemaInternal.h"

#include "openctm/openctm.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;


template <typename T> T getJsonValue (const Json::Value& pt);

template<> double getJsonValue (const Json::Value& pt)
{
	return pt.asDouble ();
}

template<> size_t getJsonValue (const Json::Value& pt)
{
	return pt.asUInt ();
}

template<> string getJsonValue (const Json::Value& pt)
{
	return pt.asCString ();
}

USING_NAMESPACE_BENTLEY_THREEMX_SCHEMA

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T> void readVector (const Json::Value& pt, bvector<T>& v)
    {
    v.clear();
    for (Json::ArrayIndex i = 0; i < pt.size (); i++)
        {
	v.push_back (getJsonValue<T>(pt[i]));
	}
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T> bool readVectorEntry (const Json::Value& pt, const string& tag, bvector<T>& v, string& err)
    {
    Json::Value entry = pt[tag.c_str ()];
    if (!entry.isArray ())
        return false;

    for (Json::ArrayIndex i = 0; i < entry.size (); i++)
	{
	v.push_back (getJsonValue<T>(entry[i]));
	}
    return true;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool readNodeInfo (const Json::Value& pt, S3NodeInfo& nodeInfo, string& nodeName, bvector<string>& nodeResources, string& err)
    {
    nodeName = pt.get ("id", "").asCString ();

    if (nodeName.empty())
	{
        err = "No node id";
	return false;
	}

    bvector<double> bbMin;
    bvector<double> bbMax;
    bvector<double> sphereCenter;
    sphereCenter.resize(3);

    double sphereDiameter;

    if (!readVectorEntry(pt, "bbMin", bbMin, err)) 
        return false;

    if (bbMin.size() != 3)
        {
	err = "Malformed bbMin";
	return false;
	}

    if (!readVectorEntry(pt, "bbMax", bbMax, err)) 
        return false;

    if (bbMax.size() != 3)
	{
	err = "Malformed bbMax";
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
        err = "Cannot find \"maxScreenDiameter\" entry";
        return false;
        }
    nodeInfo.m_dMax = val.asDouble();
    if (!readVectorEntry(pt, "resources", nodeResources, err))
        {
        err = "Cannot find \"resources\" entry";
        return false;
        }

    if (!readVectorEntry(pt, "children", nodeInfo.m_children, err))
        return false;

    return true;
}

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static CTMuint CTMCALL ctmReadFunc(void* buf, CTMuint count, void* userData)
    {
    return CTMuint(((istream*)userData)->read((char*)buf, count).gcount());
    }

static const std::string magicNumber = "3MXBO";
static const std::string sceneExtension = "3mx";
static const std::string nodeExtension = "3mxb";
static const std::string sceneVersionTag = "3mxVersion";


struct ThreeMX
    {
    static std::string getMagicNumber() { return magicNumber; }
    static std::string getSceneExtension() { return sceneExtension; }
    static std::string getNodeExtension() { return nodeExtension; }
    static std::string getSceneVersionTag() { return sceneVersionTag; }
    static std::string getLongName(){ return "Smart3DCapture 3MX"; }
    static std::string getDescription(){ return "Smart3DCapture 3D Multiresolution Mesh Exchange Format"; }
    static std::string getShortName(){ return "3MX"; }
    };

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BaseMeshNode::Read3MXB(std::istream& in, std::string& err)
    {
    _Clear();
    bmap<std::string, int>  textureIds;
    bmap<std::string, int>  nodeIds;
    bmap<std::string, std::string>  geometryNodeCorrespondence;
    int     textureCount = 0, nodeCount = 0;

    std::string magicNumber;
    magicNumber.reserve (ThreeMX::getMagicNumber ().size ());
    if (in.read(const_cast<char*>(magicNumber.c_str()), ThreeMX::getMagicNumber().size()).fail())
	{
        err = "Cannot read the magic number";
        return (BentleyStatus)ERROR;
	}

    if (strncmp(magicNumber.c_str(), ThreeMX::getMagicNumber().c_str(), ThreeMX::getMagicNumber().size()))
	{
        err = "Incorrect magic number";
        return (BentleyStatus)ERROR;
	}

    uint32_t infoSize;
    if (in.read((char*)&infoSize, 4).fail())
	{
        err = "Cannot read size";
        return (BentleyStatus)ERROR;
	}

    char* infoBuffer = new char[infoSize];
    if (in.read(infoBuffer, infoSize).fail())
	{
        err = "Cannot read info";
	delete[] infoBuffer;
        return (BentleyStatus)ERROR;
	}
    stringstream infoSS;
    infoSS.write(infoBuffer, infoSize);
    delete[] infoBuffer;

    Json::Value  pt(Json::objectValue);
    Json::Reader reader;
    if (!reader.parse(Utf8String(infoSS.str().c_str()), pt))
        {
        err = string("Cannot parse info: ");
        return (BentleyStatus)ERROR;
        }

    int version = pt.get("version", 0).asInt();

    if (version != 1)
        {
        err = "Unsupported version";
        return (BentleyStatus)ERROR;
        }

    Json::Value entry = pt["nodes"];
    if (!entry.empty())
        {
        for (Json::ArrayIndex i = 0; i < entry.size(); i++)
            {
            S3NodeInfo nodeInfo;
            string nodeName;
            bvector<string> nodeResources;

            if (!readNodeInfo(entry[i], nodeInfo, nodeName, nodeResources, err))
                return (BentleyStatus)ERROR;

            nodeIds[nodeName] = nodeCount++;
            for (size_t i = 0; i < nodeResources.size(); i++){
                geometryNodeCorrespondence[nodeResources[i]] = nodeName;
            }
        _PushNode(nodeInfo);
        }

    string resourceType;
    string resourceFormat;
    string resourceName;
    size_t resourceSize;
    size_t offset = ThreeMX::getMagicNumber().size() + 4 + infoSize;

    entry = pt["resources"];

    if (!entry.empty())
        for (Json::ArrayIndex i = 0; i < entry.size(); i++)
            {
            const Json::Value& childPt = entry[i];
            resourceType = childPt.get("type", "").asCString();
            resourceFormat = childPt.get("format", "").asCString();
            resourceName = childPt.get("id", "").asCString();
            resourceSize = (size_t)childPt.get("size", 0).asUInt();

            if (resourceType == "textureBuffer" && resourceFormat == "jpg"
                && !resourceName.empty() && resourceSize > 0)
                {
                unsigned char* buffer = new unsigned char[resourceSize];
                in.seekg(offset, in.beg);
                if (in.read((char*)buffer, resourceSize).fail())
                    {
                    err = "Cannot read child data";
                    delete[] buffer;
                    _Clear();
                    return (BentleyStatus)ERROR;
                    }

                _PushJpegTexture(buffer, resourceSize);
                textureIds[resourceName] = textureCount++;
                delete[] buffer;
                }
            else if (resourceType == "geometryBuffer" && resourceFormat == "ctm"
                    && !resourceName.empty() && resourceSize > 0)
                {
                if (geometryNodeCorrespondence.find(resourceName) == geometryNodeCorrespondence.end())
                    {
                    err = "Geometry is not referenced by any node";
                    _Clear();
                    return (BentleyStatus)ERROR;
                    }
                string nodeName = geometryNodeCorrespondence[resourceName];
                if (nodeIds.find(nodeName) == nodeIds.end())
                    {
                    err = "Node name is unknown";
                    _Clear();
                    return (BentleyStatus)ERROR;
                    }
                int nodeId = nodeIds[nodeName];

                CTMcontext context;
                context = ctmNewContext(CTM_IMPORT);
                if (ctmGetError(context) != CTM_NONE)
                    {
                    err = string("CTM context error: ") + ctmErrorString(ctmGetError(context));
                    _Clear();
                    return (BentleyStatus)ERROR;
                    }
                in.seekg(offset, in.beg);
                ctmLoadCustom(context, ctmReadFunc, &in);
                if (ctmGetError(context) != CTM_NONE)
                    {
                    err = string("CTM read error: ") + ctmErrorString(ctmGetError(context));
                    _Clear();
                    return (BentleyStatus)ERROR;
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
                    string texName;
                    texName = childPt.get("texture", Json::Value("")).asCString();
                    if (!texName.empty())
                        {
                        textureCoordinates = (float*)ctmGetFloatArray(context, CTM_UV_MAP_1);
                        if (textureIds.find(texName) == textureIds.end())
                            {
                            err = string("Bad texture name ") + texName;
                            ctmFreeContext(context);
                            _Clear();
                            return (BentleyStatus)ERROR;
                            }
                        textureId = textureIds[texName];
                        }
                    }
                _AddGeometry(nodeId, nbVertices, positions, normals, nbTriangles, indices, textureCoordinates, textureId);
                ctmFreeContext(context);
                }//end CTM
            else 
                {
                err = "Bad children definition: resourceType = " + resourceType + "; resourceFormat = " + resourceFormat + "; resourceName = " + resourceName;
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
BentleyStatus BaseMeshNode::Read3MXB(BeFileNameCR filename, std::string& err)
{
	_Clear();
	err = "";

    _SetDirectory(BeFileName(BeFileName::DevAndDir, filename.c_str()));
    ifstream in(filename.c_str(), ios::binary);


    if (!in.is_open())
        {
	err = "Cannot open file";
        return (BentleyStatus)ERROR;
	}
    return Read3MXB(in, err);
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BaseSceneNode::Read3MX(std::istream& in, S3SceneInfo& outSceneInfo, std::string& err) 
    {
    std::stringstream buffer;
    char tmp[1024];
    while (!in.eof())
	{
        in.read(tmp, 1024);
	buffer.write(tmp, in.gcount());
	}
    Json::Value  pt(Json::objectValue);
    Json::Reader reader;

    if (!reader.parse(Utf8String(buffer.str().c_str()), pt))
	{
        err = string("Cannot parse info: ");
        return (BentleyStatus)ERROR;
	}
    int version = pt.get(ThreeMX::getSceneVersionTag().c_str(), 0).asInt();

    if (version != 1)
        {
	err = "Unsupported version of " + ThreeMX::getShortName();
        return (BentleyStatus)ERROR;
	}

    outSceneInfo.sceneName = pt.get("name", "").asCString();

    Json::Value entry = pt["sceneOptions"];
    if (!entry.empty()){
        for (Json::ArrayIndex i = 0; i < entry.size(); i++)
            {
	    outSceneInfo.navigationMode = entry[i].get("navigationMode", "").asCString();
	    }
	}

    /*Json::Value */entry = pt["layers"];
    string meshType;
    string meshPath;
    if (entry.empty())
        {
        err = "Cannot find \"layers\" tag";
        return (BentleyStatus)ERROR;
	}
    else 
        {
        for (Json::ArrayIndex i = 0; i < entry.size(); i++)
            {
	    const Json::Value& childPt = entry[i];
	    meshType = childPt.get("type", "").asCString();
	    if (meshType == "meshPyramid") 
                {
		meshPath = childPt.get("root", "").asCString();
		if (meshPath.empty()) 
                    {
		    err = "Could not find the mesh path";
                    return (BentleyStatus)ERROR;
                    }
                outSceneInfo.meshChildren.push_back(meshPath);
		outSceneInfo.SRS = childPt.get("SRS", "").asCString();
                if (!outSceneInfo.SRS.empty())
                    {
                    if (!readVectorEntry(childPt, "SRSOrigin", outSceneInfo.SRSOrigin, err))
                        {
                        err = "Could not find the SRS origin";
                        return (BentleyStatus)ERROR;
                        }
                    if (outSceneInfo.SRSOrigin.size() != 3)
                        {
                        err = "Malformed SRS origin";
                        return (BentleyStatus)ERROR;
                        }
                    }
                }
            }
        }
    return (BentleyStatus)SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BaseSceneNode::Read3MX(BeFileNameCR filename, S3SceneInfo& outSceneInfo, std::string& err)
    {
    err = "";

    ifstream in(filename.c_str(), ios::binary);
    if (!in.is_open())
        {
	err = "Cannot open file";
        return (BentleyStatus)ERROR;
	}
    BentleyStatus result = Read3MX(in, outSceneInfo, err);
    in.close();
    return result;
    }
