#include <gl/glew.h>
#include "SMDisplayMgr.h"
#include <mutex>


using namespace ScalableMesh;
std::mutex elemMutex;

SMDisplayMgr::SMDisplayMgr()
    {
    m_Center = DPoint3d::From(0, 0, 0);         // The 3SM BBox center, to avoid Big coordinates handling; used for re-centering
    }

SMDisplayMgr::~SMDisplayMgr()
    {
    }

BentleyStatus SMDisplayMgr::_CreateCachedMesh(
    SmCachedDisplayMesh*&   cachedDisplayMesh,  // this display mesh will be created and filled here
    size_t                  nbVertices,         // the number of vertices for this mesh part
    DPoint3d const*         positionOrigin,     // vertex coords are given relative to this origin/node
    float*                  positions,          // list of vertex positions (nbVertices*3)
    float*                  normals,            // list of normals (not filled)
    int                     nbTriangles,        // the number of triangular faces
    int*                    indices,            // list of indices (3*nbTriangles)
    float*                  params,             // UVs, or 0 if untextured
    SmCachedDisplayTexture* cachedTexture,      // the associated texture
    uint64_t                nodeId,             // id of the SM index node
    uint64_t                smId)               // id of the scalable mesh    
    {
    // Everything here should be thread safe !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // No opengl instructions here
    //std::lock_guard<std::mutex> l(elemMutex); // uncomment this for debug

    cachedDisplayMesh = new SmCachedDisplayMesh();
    cachedDisplayMesh->nbPoints = nbVertices;
    cachedDisplayMesh->nbTriangles = nbTriangles;
    cachedDisplayMesh->meshId = smId;
	cachedDisplayMesh->nodeId = nodeId;
    cachedDisplayMesh->cachedTexture = cachedTexture; // just store the pointer

    // Get UVs information
    cachedDisplayMesh->uvs = nullptr;
    if (params != nullptr)
        {
        cachedDisplayMesh->uvs = new float[2 * nbVertices];
        memcpy(cachedDisplayMesh->uvs, params, 2 * nbVertices * sizeof(float));
        }

    // Get The Vertices
    cachedDisplayMesh->points = new DPoint3d[nbVertices];
    DVec3d recenter = DVec3d(*positionOrigin - m_Center); // coords are given relative to a local node center => recenter according to the Global Scene Center
    int n = 0;
    for (int i = 0; i < nbVertices; i++, n+=3)
        {
        cachedDisplayMesh->points[i] = DPoint3d::From(positions[n], positions[n + 1], positions[n + 2]) + recenter;
        }

    // Get the positions
    cachedDisplayMesh->localCenter = recenter; // cannot apply this transf in memcpy, store it for glTrqnslate
    cachedDisplayMesh->positions = new float[3 * nbVertices];
    memcpy(cachedDisplayMesh->positions, positions, 3 * nbVertices * sizeof(float));

    // Get Face indices
    cachedDisplayMesh->indices = new int[nbTriangles * 3];
    memcpy(cachedDisplayMesh->indices, indices, 3 * nbTriangles * sizeof(int));
 
	cachedDisplayMesh->displayIndexIBO = -1;
    return BentleyStatus::SUCCESS;
    }

//Called when node is unloaded or data becomes invalidated
BentleyStatus SMDisplayMgr::_DestroyCachedMesh(SmCachedDisplayMesh* cachedDisplayMesh)
    {

	if (s_useVBO)
	{
		if (cachedDisplayMesh->displayIndexIBO != -1)
		{
			glDeleteBuffers(1, (unsigned int*)&cachedDisplayMesh->displayIndexIBO);
			return BentleyStatus::SUCCESS;
		}
	}
    // Drawing cache update
    glDeleteLists(cachedDisplayMesh->displayIndex, 1); // delete it if it is not used any more

    // Data part
	if (!s_dontkeepIntermediateDisplayData || cachedDisplayMesh->points != nullptr)
	{
		delete[] cachedDisplayMesh->points;
		cachedDisplayMesh->points = NULL;
	}

	if (!s_dontkeepIntermediateDisplayData || cachedDisplayMesh->positions != nullptr)
	{
		delete[] cachedDisplayMesh->positions;
		cachedDisplayMesh->positions = NULL;
	}

	if (!s_dontkeepIntermediateDisplayData || cachedDisplayMesh->indices != nullptr)
	{
		delete[] cachedDisplayMesh->indices;
		cachedDisplayMesh->indices = NULL;
	}

	if (cachedDisplayMesh->uvs)
		delete[] cachedDisplayMesh->uvs;
	

    delete cachedDisplayMesh;
    cachedDisplayMesh = NULL;

    return BentleyStatus::SUCCESS;
    }

BentleyStatus SMDisplayMgr::_CreateCachedTexture(SmCachedDisplayTexture*& cachedDisplayTexture,
    int                      xSize,
    int                      ySize,
    int                      enableAlpha,
    int                      format,
    unsigned char const *    texels)
    {
    // Everything here should be thread safe !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    //std::cout << "CreateTexture :: " << format << std::endl;
    std::unique_ptr<SmCachedDisplayTexture> mCachedDisplayTexture(new SmCachedDisplayTexture);

    if (format == 2) // ==> RGB_FORMAT
        {
        mCachedDisplayTexture->width  = xSize;
        mCachedDisplayTexture->height = ySize;
        mCachedDisplayTexture->texels = new unsigned char[3*xSize*ySize];
        memcpy(mCachedDisplayTexture->texels, texels, 3*xSize*ySize * sizeof(unsigned char));
        }
    else {
        std::cerr << "!!!!! Texture format not supported :: " << format << std::endl;
        return BentleyStatus::ERROR;
        }

    cachedDisplayTexture = mCachedDisplayTexture.release();

    return BentleyStatus::SUCCESS;
    }

BentleyStatus SMDisplayMgr::_DestroyCachedTexture(SmCachedDisplayTexture* cachedDisplayTexture)
    {
    // Drawing cache update
    glDeleteTextures(1 , (GLuint*)&cachedDisplayTexture->m_textureId); // delete it if it is not used any more

    // data part release
	if (!s_dontkeepIntermediateDisplayData || cachedDisplayTexture->texels != nullptr)
	{
		delete[] cachedDisplayTexture->texels;
		cachedDisplayTexture->texels = NULL;
	}

    delete cachedDisplayTexture;
    cachedDisplayTexture = NULL;

    return BentleyStatus::SUCCESS;
    }