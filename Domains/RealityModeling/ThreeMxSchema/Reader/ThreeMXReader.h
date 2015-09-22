/*-------------------------------------------------------------------------------------+
|
|     $Source: ThreeMxSchema/Reader/ThreeMXReader.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

#include "ThreeMxSchemaInternal.h"


BEGIN_BENTLEY_THREEMX_SCHEMA_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct S3NodeInfo
{
	DPoint3d                    m_center;
	double                      m_radius;
	double                      m_dMax;
	bvector<std::string>        m_children;
};


/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct S3SceneInfo
{
	std::string                 sceneName;
	std::string                 SRS;
	bvector<double>             SRSOrigin;
	std::string                 navigationMode;
	bvector<std::string>        meshChildren;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct BaseSceneNode
{

public:
	// read .3mx file
	// failure: returns false, error message in err
	 static BentleyStatus Read3MX(std::istream& in, S3SceneInfo& outSceneInfo, std::string& err);
	 static BentleyStatus Read3MX(BeFileNameCR filename, S3SceneInfo& outSceneInfo, std::string& err);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct BaseMeshNode
{

public:
	// read .3mxb file
	// failure: returns false, error message in err
	 BentleyStatus Read3MXB(std::istream& in, std::string& err);
	 BentleyStatus Read3MXB(BeFileNameCR filename, std::string& err);

	virtual ~BaseMeshNode() {}


private:

	// Set node directory (useful to access children)
	virtual void _SetDirectory(BeFileNameCR dir) = 0;
	// clear any data
	virtual void _Clear() = 0;
	// Add a child node to some list
	virtual void  _PushNode(const S3NodeInfo& node) = 0;
	// Push the texture back to some texture vector
	virtual void _PushJpegTexture(Byte const* data, size_t dataSize) = 0;
	// Add a geometry to some list
	virtual void _AddGeometry
		(
		int nodeId,								// index between 0 and the number of pushed nodes
		int nbVertices,
		float* positions,                       // (X,Y,Z) x nbVertices
		float* normals,                         // NULL or (nX,nY,nZ) x nbVertices
		int nbTriangles,
		int* indices,                           // (i1,i2,i3) x nbTriangles where 0 <= i_k << nbVertices
		float* textureCoordinates,              // NULL or (u,v) x nbVertices where 0 <= u,v <= 1
		int textureId                           // if textureCoordinates!=NULL, index between 0 and number of pushed textures otherwise
		) = 0;
};

END_BENTLEY_THREEMX_SCHEMA_NAMESPACE
