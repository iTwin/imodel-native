// ***************************************************************************************
// Module		:	
// Version		:	1.0
// Description	:	
// Authors		:	Lee Bull
// Date			:	01 January 2010
// History		:	
// Docs			:	
// Notes		:	
// ***************************************************************************************

#ifndef _POINTSTREAMFILTER
#define _POINTSTREAMFILTER



// ***************************************************************************************
// Defines
// ***************************************************************************************



// ***************************************************************************************
// Includes
// ***************************************************************************************

// C
#include <vector>

// Local
#include <ptcloud2/indexstream.h>
#include <ptcloud2/SpatialPartitionGraph.h>
#include <pt/boundingbox.h>
#include <math/matrix_math.h>

namespace pcloud
{

	const SpatialPartitioner::NodeType	NodeTypePointStreamFilterPOD = SpatialPartitioner::NodeTypeEND + 100;

// ***************************************************************************************
// Class		:	
// Version		:	1.0
// Description	:	
// Authors		:	Lee Bull
// Date			:	01 January 2010
// History		:	
// Docs			:	
// Notes		:	
// ***************************************************************************************

#define FILENAME_DECORATION_SIZE 256

class NodePointStreamFilterPOD : public SpatialPartitioner::SpatialPartitionNode
{

public:

	const static unsigned int maxChildren = 0;

protected:

	pcloud::IndexStream		*	indexStream;
	std::wstring				filenameDecoration;

public:

	NodePointStreamFilterPOD(void)
	{
		setIndexStream(NULL);
	}

	void setIndexStream(pcloud::IndexStream *s)
	{
		indexStream = s;
	}

	bool initialize(void)
	{
															// Create an index stream object
		if(getIndexStream() == NULL)
		{
			pcloud::IndexStream *s = new IndexStream();
			if(s == NULL)
				return false;
			
			setIndexStream(s);
		}

		return true;
	}

	pcloud::IndexStream *getIndexStream(void)
	{
		return indexStream;
	}

	SpatialPartitioner::ChildIndex classify(SpatialPartitioner::Types<double>::Vector3 &point, SpatialPartitioner::Types<float>::Extents &extents)
	{
		return 0;
	}

	unsigned int classify(SpatialPartitioner::Types<double>::Vector3 &point, SpatialPartitioner::Types<float>::Extents &extents, SpatialPartitioner::ChildIndexSet &result)
	{
		return 0;
	}

	SpatialPartitioner::NodeType getType(void)
	{
		return NodeTypePointStreamFilterPOD;
	}

	SpatialPartitioner::ChildIndex getMaxChildren(void)
	{
		return 0;
	}

	void setFilenameDecoration(const wchar_t *str)
	{
		filenameDecoration = str;
	}

	const wchar_t *getFilenameDecoration(void)
	{
		return filenameDecoration.c_str();
	}

};

// ***************************************************************************************
// Class		:	
// Version		:	1.0
// Description	:	
// Authors		:	Lee Bull
// Date			:	01 January 2010
// History		:	
// Docs			:	
// Notes		:	
// ***************************************************************************************


class ContainerH1PointStreamFilterPOD : public SpatialPartitioner::ContainerH1NodeLeaf<float, NodePointStreamFilterPOD>
{

protected:


public:

};

// ***************************************************************************************
// Class		:	
// Version		:	1.0
// Description	:	
// Authors		:	Lee Bull
// Date			:	01 January 2010
// History		:	
// Docs			:	
// Notes		:	
// ***************************************************************************************

class PCLOUD_API PointStreamFilter : public SpatialPartitioner::Visitor
{

public:

	typedef SpatialPartitioner::Types<float>::Vector3		Vector3;
	typedef SpatialPartitioner::Types<float>::Vector3Set	Vector3Set;

	typedef SpatialPartitioner::NodeGrid<float, SpatialPartitioner::NodeDataNULL>	NodeGrid;
	typedef SpatialPartitioner::Container											Container;

protected:

// Pip Test Begin
mmatrix4d cloudMat;
// Pip Test End

	SpatialPartitioner::SpatialPartitionGraph<Vector3, Vector3Set>	spatialPartitionGraph;

	unsigned int		cloudCounter;

	pt::BoundingBox		rootLocalBoundingBox;

	mmatrix4d			transformWorldToRoot;


protected:

	SpatialPartitioner::NodeH getRootGridNodeChild(unsigned int cellsX, unsigned int cellsY, unsigned int cellsZ);

	template<class N> N *getRootNode(void)
	{
		SpatialPartitioner::NodeH root = spatialPartitionGraph.getRootNode();
		if(root.isValid() == false)
			return NULL;

		return dynamic_cast<N *>(root.getNode());
	}

	void setNumClouds(unsigned int num)
	{
		cloudCounter = num;
	}

	unsigned int getNumClouds(void)
	{
		return cloudCounter;
	}

	void setTransformWorldToRoot(mmatrix4d &t)
	{
		transformWorldToRoot = t;
	}

	const mmatrix4d *getTransformWorldToRoot(void)
	{
		return &transformWorldToRoot;
	}

public:

	PointStreamFilter(void)
	{
		setNumClouds(0);
	}

	~PointStreamFilter(void)
	{
		deleteAll();
	}

	bool				isEmpty						(void)										{return spatialPartitionGraph.isEmpty();}

	pt::BoundingBox	*	getRootLocalBoundingBox		(void)										{return &rootLocalBoundingBox;}
	void				setRootLocalBoundingBox		(const pt::BoundingBox &localBoundingBox)	{rootLocalBoundingBox = localBoundingBox;}

	bool				createFilterGrid			(const mmatrix4d &transform, const pt::BoundingBox &gridBoundingBox, unsigned int cellsX, unsigned int cellsY, unsigned int cellsZ, float overlapX, float overlapY, float overlapZ);

	void				setGridCellOverlap			(float x, float y, float z);
	void				getGridCellOverlap			(float &x, float &y, float &z);

	bool				autoDecorateGridFilenames	(void);

	bool				setGridCellNameDecoration	(unsigned int cellsX, unsigned int cellsY, unsigned int cellsZ, const wchar_t *name);
	const wchar_t	*	getGridCellNameDecoration	(void);

	void				deleteAll					(void);

	bool				visitNode					(SpatialPartitioner::NodeH &node);

															// Stream Interface compatibility
	bool				startStream					(const wchar_t *filename);
	bool				closeStream					(void);

	void				addCloud					(uint cloud_spec, const mmatrix4d *mat, uint ibound, uint jbound, const wchar_t*name);
	int					addGroup					(bool combine, float tolerance, bool gen_normals, float normal_quality, const wchar_t*name);

	void				addNull						(void);
	bool				addPoint					(const pt::vector3d &geomd, const ubyte *rgb, const short *intensity, const pt::vector3s *normal);

	int					numClouds					(void);
	void				rescaleIntensities			(void);
	void				restartCloudPass			(void);

	void				addPassPointColour			(const pt::vector3d &geomd, const ubyte *rgb);
	void				addPassPointIntensity		(const pt::vector3d &geomd, const short *intensity);

	int64_t				writeStreamPosition			(void);

	void				getIndexStreams				(std::vector<IndexStream *> &result);

	void				buildNormals				(bool transform, float quality);

};



} // End pcloud Namespace



#endif




