#include "ScalableMeshPCH.h"
#include <ScalableMesh/ScalableMeshFrom3MX.h>
#include <ScalableMesh/ScalableMeshLib.h>
#ifndef VANCOUVER_API
    #include <ThreeMxReader/ThreeMXReader.h>
    #include <DgnPlatform/ImageUtilities.h>
#else
    #include <Acute3d/ThreeMXReader.h>
    #include    <FreeImage/FreeImage.h>
#endif
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshCreator.h>
#include <ScalableMesh/IScalableMeshNodeCreator.h>
#include <ScalableMesh/IScalableMeshSourceCreator.h>
#include <ScalableMesh/IScalableMeshSourceImportConfig.h>
#include <ScalableMesh/IScalableMeshPolicy.h>

#include <stack>
#include <map>

using namespace std;

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

// Default GCS and progress handlers

// Default GCS handler which can just keep the SRS of 3MX, or do reprojection
class ScalableMeshFrom3MXDefaultGCSHandler : public IScalableMeshFrom3MXGCSHandler
{
public:
    ScalableMeshFrom3MXDefaultGCSHandler(GeoCoordinates::BaseGCSPtr reprojectToGCS = nullptr) // If a GCS is specified, 3D model is reprojected
    {
        // If an output GCS is specified, set it as the output GCS
        if (reprojectToGCS != nullptr)
            m_outputGCS = reprojectToGCS;
    }

    virtual bool _SetInputGCS(CharCP inputSRS)
    {
        // If the 3MX model is not georeferenced there is nothing to do
        // Just check that we did not ask for an output SRS, in which case this is an error
        WString wsInputSRS(inputSRS, false);
        if (wsInputSRS.empty())
            return (m_outputGCS == nullptr);

        // Try to create the input GCS from the SRS definition in WKT format
        m_inputGCS = GeoCoordinates::BaseGCS::CreateGCS();
        StatusInt warningStat;
        WString warningMessageString;
        StatusInt errorStat = m_inputGCS->InitFromWellKnownText(&warningStat, &warningMessageString, GeoCoordinates::BaseGCS::wktFlavorUnknown, wsInputSRS.c_str());
        if (errorStat != SUCCESS)
            return false;

        // To allow automatic conversion of elevation if applicable (e.g., from ellipsoid to geoid)
        m_inputGCS->SetReprojectElevation(true);

        // If no output GCS is specified, set it to the input GCS
        if (m_outputGCS == nullptr) m_outputGCS = m_inputGCS;

        return true;
    }

    virtual bool _Reproject(DPoint3dCR p, DPoint3dR q) const
    {
        // If input GCS and output GCS are the same, easy
        // This also covers the non-georeferenced case (m_outputGCS == m_input_GCS == nullptr)
        if (m_outputGCS == m_inputGCS)
        {
            q = p;
            return true;
        }

        // Otherwise, compute a reprojection
        GeoPoint inLatLong, outLatLong;
        if (m_inputGCS->LatLongFromCartesian(inLatLong, p) != SUCCESS)
            return false;
        if (m_inputGCS->LatLongFromLatLong(outLatLong, inLatLong, *m_outputGCS) != SUCCESS)
            return false;
        if (m_outputGCS->CartesianFromLatLong(q, outLatLong) != SUCCESS)
            return false;

        return true;
    }

    GeoCoordinates::BaseGCSPtr _GetOutputGCS() const { return m_outputGCS; }

protected:
    GeoCoordinates::BaseGCSPtr m_inputGCS, m_outputGCS;
};


// Default progress handler which does nothing
class ScalableMeshFrom3MXDefaultProgressHandler : public IScalableMeshFrom3MXProgressHandler
{
public:
    virtual bool _Progress(float progress) { return true; }
};

// We have to store the parent of nodes because we do not have access through the API
typedef std::map<IScalableMeshNodeEdit*, IScalableMeshNodeEditPtr> ParentNodeMap;

// Class handling conversion of a single 3MXB file. It implements the BaseMeshNode interface used by ThreeMXReader
#ifndef VANCOUVER_API
class Convert3MXBFile : public ThreeMxSchema::BaseMeshNode
#else
class Convert3MXBFile : public BaseMeshNode
#endif
{
public:
    // This struct stores the useful information collected about a node of a 3MXB file
    struct Convert3MXBNode
    {
        IScalableMeshNodeEditPtr scNode;    // Corresponding Scalable Mesh node
        DRange3d nodeExtent;                // Extent as read in the 3MX file and modified with reprojection
        DRange3d geomExtent;                // Actual geometric extent of this node, in the output GCS. Maybe a null range.
        bool isProxy;                       // true if the node is just a proxy that must always be replaced by children nodes
        float resolution;
        bvector<DPoint3d> vertices;
        bvector<int32_t> ptsIndices;
        bvector<DPoint2d> uvCoords;
        int64_t textureID;
        bvector<string> childrenFiles;
    };

private:
    IScalableMeshNodeCreatorPtr m_scMesh;            // Scalable Mesh
    DPoint3d m_pOrigin;                                // Origin of 3D coordinates in the 3MX model
    IScalableMeshFrom3MXGCSHandler& m_gcsHandler;    // GCS handler
    SMFrom3MXStatus m_convertStatus;                // Conversion status 
    bvector<Convert3MXBNode> m_nodeArray;            // Array of information collected about each node of the 3MXB file
    bvector<int64_t> m_textureIDArray;                // Array of texture IDs used in the 3MXB file
    ParentNodeMap& m_parentNodeMap;                 // Map from a node to its parent


#ifndef VANCOUVER_API // The virtual functions below are called while reading a 3MXB file        
    virtual void  _PushNode(const ThreeMxSchema::S3NodeInfo& nodeInfo)
#else
    virtual void  _PushNode(const S3NodeInfo& nodeInfo)
#endif
    {
        // Stop reading at the first error
        if (m_convertStatus != SMFrom3MXStatus::Success)
            return;

        // Record some information for future use
        Convert3MXBNode node;
        node.childrenFiles = nodeInfo.m_children;

        // Estimate node extent by reprojecting the corners of the range stored in the 3MX file
        node.nodeExtent = DRange3d::NullRange();

        // An empty range is encoded in 3MX by a punctual range
        if (!(nodeInfo.m_range.low == nodeInfo.m_range.high))
        {
            DPoint3d cornerArray[8];
            nodeInfo.m_range.Get8Corners(cornerArray);
            for (int i = 0; i < 8; i++)
            {
                DPoint3d p = cornerArray[i];
                p.Add(m_pOrigin); // Apply SRS origin to 3MX coordinates

                                  // Reproject point (may acutally be identity)
                DPoint3d q;
                if (!m_gcsHandler._Reproject(p, q))
                {
                    m_convertStatus = SMFrom3MXStatus::ReprojectionError;
                    return;
                }

                // Update node extent with reprojected corner
                node.nodeExtent.Extend(q);
            }
        }

        // Initialize actual geometric extent which will be set by _AddGeometry
        node.geomExtent = DRange3d::NullRange();

        // Deduce the resolution of the node from the sphere radius and the threshold
        // If the threshold is 0 (children must always be displayed), use the diameter of the bounding sphere instead
        node.resolution = (nodeInfo.m_dMax > 0) ? static_cast<float>(nodeInfo.m_radius * 2. / nodeInfo.m_dMax) : static_cast<float>(nodeInfo.m_radius * 2.);
        node.isProxy = (nodeInfo.m_dMax == 0);
        m_nodeArray.push_back(std::move(node));
    }

#ifndef VANCOUVER_API
    void _DecodeJpegData(Byte const* data, size_t dataSize, bvector<Byte>& rgb, int& width, int& height)
    {
        // Decode the JPEG buffer
        ImageUtilities::RgbImageInfo outInfo, inInfo;
        memset(&inInfo, 0, sizeof(inInfo));
        inInfo.hasAlpha = true;
        inInfo.isBGR = true;
        inInfo.isTopDown = true;

        bvector<Byte> bgra;
        if (SUCCESS != ImageUtilities::ReadImageFromJpgBuffer(bgra, outInfo, data, dataSize, inInfo))
        {
            m_convertStatus = SMFrom3MXStatus::ReadJPEGError;
            return;
        }

        // Transform the BGRA buffer into an RGB buffer
        width  = outInfo.width;
        height = outInfo.height;
        size_t nPixels = width * height;
        rgb.resize(nPixels * 3);
        for (size_t i = 0; i < nPixels; i++)
        {
            rgb[3 * i] = bgra[4 * i + 2];
            rgb[3 * i + 1] = bgra[4 * i + 1];
            rgb[3 * i + 2] = bgra[4 * i];
        }
        bgra.clear(); // Free memory asap
    }
#else
    void _DecodeJpegData(Byte const* data, size_t dataSize, bvector<Byte>& rgb, int& width, int& height)
    {
        // Decode the JPEG buffer using freeimage
        FIMEMORY*       memory = FreeImage_OpenMemory(const_cast <byte*> (data), (DWORD)dataSize);
        FIBITMAP*       bitmap;

        if (NULL != (bitmap = FreeImage_LoadFromMemory(FIF_JPEG, memory, JPEG_ACCURATE)))
        {
            width = FreeImage_GetWidth(bitmap);
            height = FreeImage_GetHeight(bitmap);

            size_t bufferSize = 3 * width * height;
            rgb.resize(bufferSize);
            byte* outP = &rgb.front();
            for (unsigned int y = 0; y < height; y++) for (unsigned int x = 0; x < width; x++)
            {
                RGBQUAD rgba;
                FreeImage_GetPixelColor(bitmap, x, height-1-y, &rgba);
                outP[0] = rgba.rgbRed;
                outP[1] = rgba.rgbGreen;
                outP[2] = rgba.rgbBlue;
                outP += 3;
            }

            FreeImage_Unload(bitmap);
        }

        FreeImage_CloseMemory(memory);
    }
#endif

    virtual void _PushJpegTexture(Byte const* data, size_t dataSize)
    {
        // Stop reading at the first error
        if (m_convertStatus != SMFrom3MXStatus::Success)
            return;

        bvector<Byte> rgb;
        int w;
        int h;

        _DecodeJpegData(data, dataSize, rgb, w, h);

        // Add the texture to the Scalable Mesh and record its id
        int64_t texID = m_scMesh->AddTexture(w, h, 3, rgb.data());
        m_textureIDArray.push_back(texID);
    }

    virtual void _Clear()
    {
        // Useless since we do not reuse an instance of this class for different 3MXB files
    }

    virtual void _AddGeometry
    (
        int nodeId,                                // index between 0 and the number of pushed nodes
        int nbVertices,
        float* positions,                       // (X,Y,Z) x nbVertices
        float* normals,                         // NULL or (nX,nY,nZ) x nbVertices
        int nbTriangles,
        int* indices,                           // (i1,i2,i3) x nbTriangles where 0 <= i_k << nbVertices
        float* textureCoordinates,              // NULL or (u,v) x nbVertices where 0 <= u,v <= 1
        int textureId                           // if textureCoordinates!=NULL, index between 0 and number of pushed textures otherwise
    )
    {
        // Stop reading at the first error
        if (m_convertStatus != SMFrom3MXStatus::Success)
            return;

        // Retrieve the Scalable Mesh node to which we must set this geometry
        if (nodeId < 0 || nodeId >= m_nodeArray.size())
        {
            m_convertStatus = SMFrom3MXStatus::Read3MXBError;
            return;
        }
        Convert3MXBNode& node = m_nodeArray[nodeId];

        // Skip the geometries corresponding to proxies
        if (node.isProxy)
        {
            node.geomExtent = DRange3d::NullRange();
            node.textureID = -1;
            return;
        }

        // If there is already a geometry, return with an error. Scalable Mesh does not support several geometries per node
        if (!node.vertices.empty())
        {
            m_convertStatus = SMFrom3MXStatus::SeveralGeometriesError;
            return;
        }

        // Convert vertices and indices to a bvector of the correct type
        // Also compute the extent of the mesh
        node.geomExtent = DRange3d::NullRange();
        node.vertices.resize(nbVertices);
        for (size_t i = 0; i < node.vertices.size(); i++)
        {
            DPoint3d& p = node.vertices[i];
            float x = *positions++, y = *positions++, z = *positions++;
            p = DPoint3d::From(x, y, z);
            p.Add(m_pOrigin); // Apply SRS origin to 3MX coordinates

                              // Reproject point (may acutally be identity)
            DPoint3d q;
            if (!m_gcsHandler._Reproject(p, q))
            {
                m_convertStatus = SMFrom3MXStatus::ReprojectionError;
                return;
            }
            p = q;

            node.geomExtent.Extend(p);
        }

        node.ptsIndices.resize(3 * nbTriangles);
        for (size_t i = 0; i < node.ptsIndices.size(); i++)
        {
            node.ptsIndices[i] = (*indices++) + 1; // Indices start at 1 in ScalableMesh
        }

        // Handle differently textured and untextured meshes
        if (textureCoordinates)
        {
            // Convert uvs to a bvector of the correct type
            node.uvCoords.resize(nbVertices);
            for (size_t i = 0; i < node.uvCoords.size(); i++)
            {
                float u = *textureCoordinates++, v = *textureCoordinates++;
                node.uvCoords[i] = DPoint2d::From(u, v);
            }

            // Retrieve the texture ID
            if (textureId < 0 || textureId >= m_textureIDArray.size())
            {
                m_convertStatus = SMFrom3MXStatus::Read3MXBError;
                return;
            }

            node.textureID = m_textureIDArray[textureId];
        }
        else
        {
            node.textureID = -1;
        }
    }

    virtual void _SetDirectory(BeFileNameCR dir) {}

    // Enforce that node extent of a node and its ancestors contain a certain range
    SMFrom3MXStatus fixNodeExtents(IScalableMeshNodeEditPtr scNode, DRange3d& range)
    {
        // If the node extent does not contain the range
        DRange3d nodeExtent = scNode->GetNodeExtent();
        if (!range.IsContained(nodeExtent))
        {
            // Modify the node extent
            nodeExtent.Extend(range);
            if (scNode->SetNodeExtent(nodeExtent) != SUCCESS)
                return SMFrom3MXStatus::ScalableMeshSDKError;

            // Check the same for the parent if not root
            auto it = m_parentNodeMap.find(scNode.get());
            if (it != m_parentNodeMap.end())
                return fixNodeExtents(it->second, range);
        }

        return SMFrom3MXStatus::Success;
    }

public:
    // Constructor
    Convert3MXBFile(IScalableMeshNodeCreatorPtr scMesh, DPoint3dCR pOrigin, IScalableMeshFrom3MXGCSHandler& gcsHandler, ParentNodeMap& parentNodeMap) : m_scMesh(scMesh), m_pOrigin(pOrigin), m_gcsHandler(gcsHandler), m_parentNodeMap(parentNodeMap), m_convertStatus(SMFrom3MXStatus::Success) {}

    // Create the 3SM nodes read from the 3MXB file
    SMFrom3MXStatus Create3SMNodes(IScalableMeshNodeEditPtr scParentNode, IScalableMeshNodeEditPtr& scRootNode)
    {
        // Stop reading at the first error
        if (m_convertStatus != SMFrom3MXStatus::Success)
            return m_convertStatus;

        // If there is no parent node and there are several nodes to create, create an additional root node
        StatusInt status;
        if (m_nodeArray.size() > 1 && scParentNode == nullptr)
        {
            scParentNode = scRootNode = m_scMesh->AddNode(status);
            if (status != SUCCESS)
                return SMFrom3MXStatus::ScalableMeshSDKError;

            // Compute the union of the extents of the nodes
            DRange3d nodeExtent = DRange3d::NullRange();
            for (Convert3MXBNode& node : m_nodeArray)
                nodeExtent.Extend(node.nodeExtent);

            // Set the node extent of this new root node
            if (scParentNode->SetNodeExtent(nodeExtent) != SUCCESS)
                return SMFrom3MXStatus::ScalableMeshSDKError;

            // Set its resolution
            float fResolution = static_cast<float>(sqrt(nodeExtent.ExtentSquared()));
            if (scParentNode->SetResolution(fResolution, fResolution) != SUCCESS)
                return SMFrom3MXStatus::ScalableMeshSDKError;
        }

        // Create the different nodes
        for (Convert3MXBNode& node : m_nodeArray)
        {
            // Fix the content extent of the parent if necessary, before adding the child
            if (scParentNode != nullptr)
            {
                SMFrom3MXStatus tmpStatus = fixNodeExtents(scParentNode, node.nodeExtent);
                if (tmpStatus != SMFrom3MXStatus::Success)
                    return tmpStatus;
            }

            // Create the root node or add a child node
            ScalableMesh::IScalableMeshNodeEditPtr scNode;
            if (scParentNode == nullptr)
            {
                scNode = m_scMesh->AddNode(status);
                scRootNode = scNode;
            }
            else
            {
                scNode = m_scMesh->AddNode(scParentNode, node.nodeExtent, status);
                if (scNode != nullptr)
                    m_parentNodeMap[scNode.get()] = scParentNode; // Store parent node
            }

            // We handle errors differently depending on whether the node extent is empty or not
            if (node.nodeExtent.IsNull())
            {
                // We are more tolerant in this case since the node extent is empty...
                if (scNode == nullptr)
                    return SMFrom3MXStatus::ScalableMeshSDKError;
            }
            else
            {
                if (status != SUCCESS)
                    return SMFrom3MXStatus::ScalableMeshSDKError;
            }

            // Set node and content extent
            // Unfortunately, 3MX does not store the tight box used during subdivision of the tree, so we just set the node extent to the extent of the 3MX node
            // These overlapping node extents may create performance problems or worse in Scalable Mesh
            if (scNode->SetNodeExtent(node.nodeExtent) != SUCCESS)
                return SMFrom3MXStatus::ScalableMeshSDKError;

            // Record the actual geometric extent of the node in the content extent. Later, we will have to update these content extents to ensure that a parent content extent contains its children's content extent
            if (!node.geomExtent.IsNull() && scNode->SetContentExtent(node.geomExtent) != SUCCESS)
                return SMFrom3MXStatus::ScalableMeshSDKError;

            // Set geometric and texture resolution
            if (scNode->SetResolution(node.resolution, node.resolution) != SUCCESS)
                return SMFrom3MXStatus::ScalableMeshSDKError;

            // Handle the case of an empty geometry
            if (!node.vertices.empty())
            {
                if (node.textureID >= 0)
                {
                    if (scNode->AddTexturedMesh(node.vertices, node.ptsIndices, node.uvCoords, node.ptsIndices, 1, node.textureID) != SUCCESS)
                        return SMFrom3MXStatus::ScalableMeshSDKError;
                }
                else
                {
                    if (scNode->AddMesh(node.vertices.data(), node.vertices.size(), node.ptsIndices.data(), node.ptsIndices.size()) != SUCCESS)
                        return SMFrom3MXStatus::ScalableMeshSDKError;
                }
            }

            // Record the created Scalable Mesh node
            node.scNode = scNode;

            // Clear memory which is not needed anymore
            node.vertices.clear();
            node.ptsIndices.clear();
            node.uvCoords.clear();
        }

        return SMFrom3MXStatus::Success;
    }

    // Access to information about the created nodes
    const bvector<Convert3MXBNode>& getNodes() const { return m_nodeArray; }
};


// Class handling conversion of a 3MX model
class Convert3MXModel
{
private:
    IScalableMeshNodeCreatorPtr m_scMesh;
    IScalableMeshNodeEditPtr m_scRootNode;
    DPoint3d m_pOrigin;
    IScalableMeshFrom3MXGCSHandler& m_gcsHandler;
    IScalableMeshFrom3MXProgressHandler& m_progressHandler;
    ParentNodeMap m_parentNodeMap;

    SMFrom3MXStatus ConvertRecursive(BeFileName filename, IScalableMeshNodeEditPtr scParentNode, DRange3d& contentExtent, float beginProgress, float endProgress)
    {
        // Report progress
        if (!m_progressHandler._Progress(beginProgress))
            return SMFrom3MXStatus::Canceled;

        // Count the number of children files, just to correctly track progress
        size_t nChildrenFiles = 0;

        // Copy the node information which is really needed for recursion
        struct NodeInfoForRecursion
        {
            bvector<BeFileName> childrenFileArray;
            DRange3d geomExtent;
            IScalableMeshNodeEditPtr scNode;
        };
        bvector<NodeInfoForRecursion> nodeArray;

        // Isolate this in a block to avoid useless memory consumption in the recursion
        {
            // Read the 3MXB file
            Convert3MXBFile conv3MXB(m_scMesh, m_pOrigin, m_gcsHandler, m_parentNodeMap);
            string sError;
            if (conv3MXB.Read3MXB(filename, sError) != SUCCESS)
                return SMFrom3MXStatus::Read3MXBError;

            // Create the corresponding 3SM nodes
            SMFrom3MXStatus convertStatus = conv3MXB.Create3SMNodes(scParentNode, m_scRootNode);
            if (convertStatus != SMFrom3MXStatus::Success)
                return convertStatus;

            // Directory to build the filenames of the child 3MXB files
            WString dir = BeFileName::GetDirectoryName(filename);

            // Copy what we really need for the recursion
            auto tmpNodeArray = conv3MXB.getNodes();
            nodeArray.resize(tmpNodeArray.size());
            for (int i = 0; i < nodeArray.size(); i++)
            {
                const auto& tmpNodeInfo = tmpNodeArray[i];
                nodeArray[i].geomExtent = tmpNodeInfo.geomExtent;
                nodeArray[i].scNode = tmpNodeInfo.scNode;

                // If node as an empty node extent, we don't traverse the children files (there shouldn't be any child anyway, but we never know ;-))
                if (!tmpNodeInfo.nodeExtent.IsNull())
                {
                    for (const string& childFile : tmpNodeInfo.childrenFiles)
                    {
                        BeFileName child3MXBPath(dir.c_str());
                        child3MXBPath.AppendToPath(BeFileName(childFile.c_str(), false));
                        nodeArray[i].childrenFileArray.push_back(child3MXBPath);
                        nChildrenFiles++;
                    }
                }
            }
        }

        // Initialize the content extent of the nodes contained in the current 3MXB file
        contentExtent = DRange3d::NullRange();

        // Traverse the children files
        size_t iChildFile = 0;

        for (NodeInfoForRecursion& node : nodeArray)
        {
            // We will compute the content extent of the node by union of the actual geometric extent of the node with the union of such for all children nodes
            DRange3d nodeContentExtent = node.geomExtent;

            for (const BeFileName& child3MXBPath : node.childrenFileArray)
            {
                // Determine the progress interval for the child 3MXB file
                float deltaProgress = (endProgress - beginProgress) / nChildrenFiles;
                float beginChildProgress = beginProgress + iChildFile * deltaProgress;
                float endChildProgress = beginChildProgress + deltaProgress;

                // Traverse the child files
                DRange3d childContentExtent = DRange3d::NullRange();
                SMFrom3MXStatus convertStatus = ConvertRecursive(child3MXBPath, node.scNode, childContentExtent, beginChildProgress, endChildProgress);
                if (convertStatus != SMFrom3MXStatus::Success)
                    return convertStatus;

                // Update the content extent of the node
                nodeContentExtent.Extend(childContentExtent);

                iChildFile++;
            }

            // Set the content extent of the node
            if (!nodeContentExtent.IsNull() && node.scNode->SetContentExtent(nodeContentExtent) != SUCCESS)
                return SMFrom3MXStatus::ScalableMeshSDKError;

            // Update the content extent of the 3MXB file
            contentExtent.Extend(nodeContentExtent);
        }

        // If there is no parent node and an additional root node was created, set its content extent
        if (nodeArray.size() > 1 && scParentNode == nullptr)
        {
            if (!contentExtent.IsNull() && m_scRootNode->SetContentExtent(contentExtent) != SUCCESS)
                return SMFrom3MXStatus::ScalableMeshSDKError;
        }

        return SMFrom3MXStatus::Success;
    }

    SMFrom3MXStatus CreateSource(BeFileNameCR input3MXPath, BeFileNameCR output3SMPath)
    {
        // Open the 3SM file for creation of a source
        StatusInt status;
        IScalableMeshSourceCreatorPtr scMesh = ScalableMesh::IScalableMeshSourceCreator::GetFor(output3SMPath.c_str(), status);
        if (status != SUCCESS) return SMFrom3MXStatus::ScalableMeshSDKError;

        // Create a source with the path of the 3MX file
        IDTMSourcePtr pSource = IDTMLocalFileSource::Create(ScalableMesh::DTM_SOURCE_DATA_MESH, input3MXPath.c_str());
        SourceImportConfig& config = pSource->EditConfig();
        Import::ScalableMeshData data = config.GetReplacementSMData();
        data.SetRepresenting3dData(true);
        config.SetReplacementSMData(data);

        // Add the source to the Scalable Mesh
        status = scMesh->EditSources().Add(pSource);
        if (status != SUCCESS) return SMFrom3MXStatus::ScalableMeshSDKError;

        // Save the changes to the file
        status = scMesh->SaveToFile();
        if (status != SUCCESS) return SMFrom3MXStatus::ScalableMeshSDKError;

        return SMFrom3MXStatus::Success;
    }

public:
    // Constructor
    Convert3MXModel(IScalableMeshFrom3MXGCSHandler& gcsHandler, IScalableMeshFrom3MXProgressHandler& progressHandler) : m_gcsHandler(gcsHandler), m_progressHandler(progressHandler) {}

    // Run conversion
    SMFrom3MXStatus Convert(BeFileNameCR input3MXPath, BeFileNameCR output3SMPath)
    {
        if (!m_progressHandler._Progress(0.f))
            return SMFrom3MXStatus::Canceled;

        // Read scene file

#ifndef VANCOUVER_API
        ThreeMxSchema::S3SceneInfo sceneInfo;
        string sError;
        if (ThreeMxSchema::BaseSceneNode::Read3MX(input3MXPath, sceneInfo, sError) != SUCCESS)
            return SMFrom3MXStatus::Read3MXError;
#else
        S3SceneInfo sceneInfo;
        string sError;
        if (BaseSceneNode::Read3MX(input3MXPath, sceneInfo, sError) != SUCCESS)
            return SMFrom3MXStatus::Read3MXError;
#endif

        // Inform GCS handler of the input SRS
        if (!m_gcsHandler._SetInputGCS(sceneInfo.SRS.c_str()))
            return SMFrom3MXStatus::GCSError;

        // Origin to apply to coordinates in 3MX
        m_pOrigin = DPoint3d::FromZero();
        if (sceneInfo.SRSOrigin.size() == 3)
        {
            m_pOrigin = DPoint3d::From(sceneInfo.SRSOrigin[0], sceneInfo.SRSOrigin[1], sceneInfo.SRSOrigin[2]);
        }

        // Create Scalable Mesh at output path
        StatusInt status;
        m_scMesh = IScalableMeshNodeCreator::GetFor(output3SMPath.c_str(), status);
        if (status != SUCCESS)
            return SMFrom3MXStatus::ScalableMeshSDKError;

        // Set GCS of Scalable Mesh
        GeoCoordinates::BaseGCSPtr outputGCS = m_gcsHandler._GetOutputGCS();
        if (outputGCS != nullptr && m_scMesh->SetBaseGCS(outputGCS) != SUCCESS)
            return SMFrom3MXStatus::ScalableMeshSDKError;

        // If there is no mesh, we are done
        if (sceneInfo.meshChildren.empty())
            return SMFrom3MXStatus::Success;

        // If there is more than one mesh, we cannot handle the conversion
        if (sceneInfo.meshChildren.size() > 1)
            return SMFrom3MXStatus::SeveralLayersError;

        BeFileName root3MXBPath(BeFileName::GetDirectoryName(input3MXPath).c_str());
        root3MXBPath.AppendToPath(BeFileName(sceneInfo.meshChildren[0].c_str(), false));

        DRange3d contentExtent;
        SMFrom3MXStatus convertStatus = ConvertRecursive(root3MXBPath, nullptr, contentExtent, 0.f, 0.95f);
        if (convertStatus != SMFrom3MXStatus::Success)
            return convertStatus;

        // Force nodes and Scalable Mesh to be persisted
        m_parentNodeMap.clear();
        m_scRootNode = nullptr;
        m_scMesh = nullptr;

        // Set the source
        convertStatus = CreateSource(input3MXPath, output3SMPath);
        if (convertStatus != SMFrom3MXStatus::Success)
            return convertStatus;

        if (!m_progressHandler._Progress(1.f))
            return SMFrom3MXStatus::Canceled;

        return SMFrom3MXStatus::Success;
    }
};


SMFrom3MXStatus createScalableMeshFrom3MX
(
    BeFileNameCR input3MXPath,
    BeFileNameCR output3SMPath,
    IScalableMeshFrom3MXGCSHandler& gcsHandler,
    IScalableMeshFrom3MXProgressHandler& progressHandler
)
{
    Convert3MXModel  converter(gcsHandler, progressHandler);
    return converter.Convert(input3MXPath, output3SMPath);
}


SMFrom3MXStatus createScalableMeshFrom3MX
(
    BeFileNameCR input3MXPath,
    BeFileNameCR output3SMPath,
    GeoCoordinates::BaseGCSPtr outputGCS,
    IScalableMeshFrom3MXProgressHandler& progressHandler
)
{
    ScalableMeshFrom3MXDefaultGCSHandler gcsHandler(outputGCS);
    return createScalableMeshFrom3MX(input3MXPath, output3SMPath, gcsHandler, progressHandler);
}


SMFrom3MXStatus createScalableMeshFrom3MX
(
    BeFileNameCR input3MXPath,
    BeFileNameCR output3SMPath,
    IScalableMeshFrom3MXGCSHandler& gcsHandler
)
{
    ScalableMeshFrom3MXDefaultProgressHandler progressHandler;
    return createScalableMeshFrom3MX(input3MXPath, output3SMPath, gcsHandler, progressHandler);
}


SMFrom3MXStatus createScalableMeshFrom3MX
(
    BeFileNameCR input3MXPath,
    BeFileNameCR output3SMPath,
    GeoCoordinates::BaseGCSPtr outputGCS
)
{
    ScalableMeshFrom3MXDefaultGCSHandler gcsHandler(outputGCS);
    ScalableMeshFrom3MXDefaultProgressHandler progressHandler;
    return createScalableMeshFrom3MX(input3MXPath, output3SMPath, gcsHandler, progressHandler);
}

END_BENTLEY_SCALABLEMESH_NAMESPACE
