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



// ***************************************************************************************
// Defines
// ***************************************************************************************



// ***************************************************************************************
// Includes
// ***************************************************************************************


#include "PointoolsVortexAPIInternal.h"
// Local
#include <ptcloud2/PointStreamFilter.h>


namespace pcloud
{


bool PointStreamFilter::createFilterGrid(const mmatrix4d &trans, const pt::BoundingBox &gridBoundingBox, unsigned int cellsX, unsigned int cellsY, unsigned int cellsZ, float overlapX, float overlapY, float overlapZ)
{
															// Create a new grid node
	SpatialPartitioner::Container	*	root;
	NodeGrid	*	grid;
															// If a filter hierarchy exists already, delete it
	if(spatialPartitionGraph.isEmpty() == false)
		deleteAll();

	try
	{
															// Create a root container
		if((root = spatialPartitionGraph.createContainer<SpatialPartitioner::ContainerH1Grid<float, SpatialPartitioner::NodeDataNULL> >()) == NULL)
			throw false;
															// Create a new Grid root node
		if((grid = dynamic_cast<NodeGrid *>(root->getNode(0))) == NULL)
			throw false;
															// Set the number of cells in the grid
		grid->setNumCells(dynamic_cast<NodeGrid::ContainerH1 *>(root), cellsX, cellsY, cellsZ);
															// Set the cell overlap distance
		grid->setOverlapDistance(overlapX, overlapY, overlapZ);

		SpatialPartitioner::ChildIndex c;
															// Create POD writing leaves
		for(c = 0; c < root->getMaxChildren(); c++)
		{
			ContainerH1PointStreamFilterPOD * p;
															// Create leaf for writing POD stream
			if((p = spatialPartitionGraph.createContainer<ContainerH1PointStreamFilterPOD>()))
			{
															// Set leaf stream to non root
				NodePointStreamFilterPOD *pod = dynamic_cast<NodePointStreamFilterPOD *>(p->getNode(0));
				if(pod)
				{
					pod->initialize();
				}
				else
				{
					return false;
				}
															// Add POD to grid cell child
				root->setChild(c, p);
			}
			else
			{
															// An error occured so exit
				return false;
			}
		}
	}
	catch(bool error)
	{
															// Delete anything created
		if(root)
			spatialPartitionGraph.deleteAll();

		return error;
	}

															// Set grid as hierarchy root
	spatialPartitionGraph.setRoot(root);
															// Set default name decoration
	autoDecorateGridFilenames();

															// Calculate World to Root transform
	mmatrix4d worldToRoot = trans;
	worldToRoot.invert();
															// Set World to Root coordinate system transform
	setTransformWorldToRoot(worldToRoot);

	pt::BoundingBox localCentralBoundingBox = gridBoundingBox;
	localCentralBoundingBox.centralize();
															// Set spatial partition graph root bounding box
	setRootLocalBoundingBox(localCentralBoundingBox);

	
															// Return OK
	return true;
}


bool PointStreamFilter::visitNode(SpatialPartitioner::NodeH &node)
{
	return true;
}

SpatialPartitioner::NodeH PointStreamFilter::getRootGridNodeChild(unsigned int cellsX, unsigned int cellsY, unsigned int cellsZ)
{
	SpatialPartitioner::NodeH	child;

	NodeGrid *grid = getRootNode<NodeGrid>();
	if(grid == NULL)
		return child;

	SpatialPartitioner::ChildIndex childIndex = grid->getChildIndex(cellsX, cellsY, cellsZ);

	SpatialPartitioner::NodeH root = spatialPartitionGraph.getRootNode();

	child = root.getChild(childIndex);

	return child;
}

bool PointStreamFilter::setGridCellNameDecoration(unsigned int cellsX, unsigned int cellsY, unsigned int cellsZ, const wchar_t *name)
{
	SpatialPartitioner::NodeH child = getRootGridNodeChild(cellsX, cellsY, cellsZ);

	if(child.isValid() == false)
		return false;

	SpatialPartitioner::NodeHSet result;

	spatialPartitionGraph.listDepthFirst(result, child, NodeTypePointStreamFilterPOD);

	unsigned int t;

	for(t = 0; t < result.size(); t++)
	{
		NodePointStreamFilterPOD *n = result[t].getNode<NodePointStreamFilterPOD>();
		if(n)
		{
			n->setFilenameDecoration(name);
		}
	}


	return true;
}

bool PointStreamFilter::startStream(const wchar_t *filename)
{
	SpatialPartitioner::NodeHSet	set;

    SpatialPartitioner::NodeH rootNode = spatialPartitionGraph.getRootNode();
	spatialPartitionGraph.listDepthFirst(set, rootNode, NodeTypePointStreamFilterPOD);

	// Generate a list of all POD nodes
	// call startStream(filename) on each node

	unsigned int t;

	ptds::FilePath	path(filename);

	path.stripExtension();

	for(t = 0; t < set.size(); t++)
	{
		SpatialPartitioner::NodeH	node = set[t];

		NodePointStreamFilterPOD *pod = dynamic_cast<NodePointStreamFilterPOD *>(node.getNode());

		std::wstring s(path.path());
		s.append(pod->getFilenameDecoration());
		ptds::FilePath p = s.c_str();
		p.setExtension(L".pod");

		pod->getIndexStream()->startStream(p.path());
	}
															// Initialize number of clouds to zero
	setNumClouds(0);

	return true;
}

bool PointStreamFilter::closeStream(void)
{
	unsigned int					t;
	SpatialPartitioner::NodeHSet	set;

    SpatialPartitioner::NodeH rootNode = spatialPartitionGraph.getRootNode();
	spatialPartitionGraph.listDepthFirst(set, rootNode, NodeTypePointStreamFilterPOD);

	for(t = 0; t < set.size(); t++)
	{
		SpatialPartitioner::NodeH	node = set[t];

		NodePointStreamFilterPOD *pod = dynamic_cast<NodePointStreamFilterPOD *>(node.getNode());

		if (!pod->getIndexStream()->closeStream())
			return false;
	}

	return true;
}



bool PointStreamFilter::autoDecorateGridFilenames(void)
{
	NodeGrid *grid = getRootNode<NodeGrid>();
	if(grid == NULL)
		return false;

	SpatialPartitioner::ChildIndex size[3];

	grid->getNumCells(size[0], size[1], size[2]);

	unsigned int x, y, z;

	char buffer[128];

	for(x = 0; x < size[0]; x++)
	{
		for(y = 0; y < size[1]; y++)
		{
			for(z = 0; z < size[2]; z++)
			{
				sprintf(buffer, "_%d_%d_%d", x, y, z);

				ptds::FilePath	p(buffer);

				setGridCellNameDecoration(x, y, z, p.filename());
			}
		}
	}

	return true;
}



void PointStreamFilter::addCloud(uint cloud_spec, const mmatrix4d *mat, uint ibound, uint jbound, const wchar_t*name)
{
	unsigned int					t;
	SpatialPartitioner::NodeHSet	set;

	// NOTE: clear cloud_spec flag for uniform grid ???
	ibound = 0;
	jbound = 0;
	cloud_spec &= ~PCLOUD_CLOUD_ORDERED;

	if(mat)
	{
		cloudMat = *mat;
	}
	else
	{
		cloudMat = mmatrix4d::identity();
	}

    SpatialPartitioner::NodeH rootNode = spatialPartitionGraph.getRootNode();
	spatialPartitionGraph.listDepthFirst(set, rootNode, NodeTypePointStreamFilterPOD);

	for(t = 0; t < set.size(); t++)
	{
		SpatialPartitioner::NodeH	node = set[t];

		NodePointStreamFilterPOD *pod = dynamic_cast<NodePointStreamFilterPOD *>(node.getNode());

		pod->getIndexStream()->addCloud(cloud_spec, mat, ibound, jbound, name);
	}

	setNumClouds(getNumClouds() + 1);
}


int PointStreamFilter::addGroup(bool combine, float tolerance, bool gen_normals, float normal_quality, const wchar_t*name)
{
	unsigned int					t;
	SpatialPartitioner::NodeHSet	set;

    SpatialPartitioner::NodeH rootNode = spatialPartitionGraph.getRootNode();
	spatialPartitionGraph.listDepthFirst(set, rootNode, NodeTypePointStreamFilterPOD);

	for(t = 0; t < set.size(); t++)
	{
		SpatialPartitioner::NodeH	node = set[t];

		NodePointStreamFilterPOD *pod = dynamic_cast<NodePointStreamFilterPOD *>(node.getNode());

		pod->getIndexStream()->addGroup(combine, tolerance, gen_normals, normal_quality, name);
	}

															// Return zero as groupidx doesn't make sense for multiple streams
	return 0;
}

void PointStreamFilter::addNull(void)
{
	unsigned int					t;
	SpatialPartitioner::NodeHSet	set;

    SpatialPartitioner::NodeH rootNode = spatialPartitionGraph.getRootNode();
	spatialPartitionGraph.listDepthFirst(set, rootNode, NodeTypePointStreamFilterPOD);

	for(t = 0; t < set.size(); t++)
	{
		SpatialPartitioner::NodeH	node = set[t];

		NodePointStreamFilterPOD *pod = dynamic_cast<NodePointStreamFilterPOD *>(node.getNode());

		pod->getIndexStream()->addNull();
	}

}


bool PointStreamFilter::addPoint(const pt::vector3d &geomd, const ubyte *rgb, const short *intensity, const pt::vector3s *normal)
{
	SpatialPartitioner::NodeHSet receivers;
															// Transform geom point to local root's coordinate system
	SpatialPartitioner::Types<double>::Vector4	v(geomd.x, geomd.y, geomd.z, 1);
	SpatialPartitioner::Types<double>::Vector4	vTemp1;
	SpatialPartitioner::Types<double>::Vector4	vTemp2;
	vTemp1 = cloudMat >> v;
	vTemp2 = (*getTransformWorldToRoot()) >> vTemp1;
	SpatialPartitioner::Types<double>::Vector3	vLocal(vTemp2(0, 0), vTemp2(1, 0), vTemp2(2, 0));

	unsigned int t;

															// Filter to receiver based on point geometry
	spatialPartitionGraph.classify(vLocal, *getRootLocalBoundingBox(), receivers);

	for(t = 0; t < receivers.size(); t++)
	{
		SpatialPartitioner::NodeH receiver = receivers[t];

		if(receiver.isValid())
		{
			NodePointStreamFilterPOD *node;

			if((node = receiver.getNode<NodePointStreamFilterPOD>()) != NULL)
			{
				node->getIndexStream()->addPoint(geomd, rgb, intensity, normal, 0/*todo: support classification*/);
			}
		}

	}

	return true;
}

int PointStreamFilter::numClouds(void)
{
	return getNumClouds();
}

void PointStreamFilter::rescaleIntensities(void)
{
	unsigned int					t;
	SpatialPartitioner::NodeHSet	set;

    SpatialPartitioner::NodeH rootNode = spatialPartitionGraph.getRootNode();
	spatialPartitionGraph.listDepthFirst(set, rootNode, NodeTypePointStreamFilterPOD);

	for(t = 0; t < set.size(); t++)
	{
		SpatialPartitioner::NodeH	node = set[t];

		NodePointStreamFilterPOD *pod = dynamic_cast<NodePointStreamFilterPOD *>(node.getNode());

		pod->getIndexStream()->rescaleIntensities();
	}

}

void PointStreamFilter::restartCloudPass(void)
{
	unsigned int					t;
	SpatialPartitioner::NodeHSet	set;

    SpatialPartitioner::NodeH rootNode = spatialPartitionGraph.getRootNode();
	spatialPartitionGraph.listDepthFirst(set, rootNode, NodeTypePointStreamFilterPOD);

	for(t = 0; t < set.size(); t++)
	{
		SpatialPartitioner::NodeH	node = set[t];

		NodePointStreamFilterPOD *pod = dynamic_cast<NodePointStreamFilterPOD *>(node.getNode());

		pod->getIndexStream()->restartCloudPass();
	}

}


int64_t PointStreamFilter::writeStreamPosition(void)
{
	unsigned int					t;
	SpatialPartitioner::NodeHSet	set;

    SpatialPartitioner::NodeH rootNode = spatialPartitionGraph.getRootNode();
	spatialPartitionGraph.listDepthFirst(set, rootNode, NodeTypePointStreamFilterPOD);

	for(t = 0; t < set.size(); t++)
	{
		SpatialPartitioner::NodeH	node = set[t];

		NodePointStreamFilterPOD *pod = dynamic_cast<NodePointStreamFilterPOD *>(node.getNode());

		pod->getIndexStream()->writeStreamPosition();
	}

	return 0;
}


void PointStreamFilter::addPassPointColour(const pt::vector3d &geomd, const ubyte *rgb)
{
	SpatialPartitioner::NodeH	receiver;

															// Transform geom point to local root's coordinate system
	SpatialPartitioner::Types<double>::Vector4	v(geomd.x, geomd.y, geomd.z, 1);
	SpatialPartitioner::Types<double>::Vector4	vTemp1;
	SpatialPartitioner::Types<double>::Vector4	vTemp2;
	vTemp1 = cloudMat >> v;
	vTemp2 = (*getTransformWorldToRoot()) >> vTemp1;
	SpatialPartitioner::Types<double>::Vector3	vLocal(vTemp2(0, 0), vTemp2(1, 0), vTemp2(2, 0));

															// Filter to receiver based on point geometry
	receiver = spatialPartitionGraph.classify(vLocal, *getRootLocalBoundingBox());
															// If point filtered to a leaf receiver
	if(receiver.isValid())
	{
		NodePointStreamFilterPOD *node;

		if((node = receiver.getNode<NodePointStreamFilterPOD>()) != NULL)
		{
			node->getIndexStream()->addPassPointColour(/*geomd,*/ rgb);
		}
	}

}

void PointStreamFilter::addPassPointIntensity(const pt::vector3d &geomd, const short *intensity)
{
	SpatialPartitioner::NodeH	receiver;

	// Transform geom point to local root's coordinate system
	SpatialPartitioner::Types<double>::Vector4	v(geomd.x, geomd.y, geomd.z, 1);
	SpatialPartitioner::Types<double>::Vector4	vTemp1;
	SpatialPartitioner::Types<double>::Vector4	vTemp2;
	vTemp1 = cloudMat >> v;
	vTemp2 = (*getTransformWorldToRoot()) >> vTemp1;
	SpatialPartitioner::Types<double>::Vector3	vLocal(vTemp2(0, 0), vTemp2(1, 0), vTemp2(2, 0));

	// Filter to receiver based on point geometry
	receiver = spatialPartitionGraph.classify(vLocal, *getRootLocalBoundingBox());
	// If point filtered to a leaf receiver
	if(receiver.isValid())
	{
		NodePointStreamFilterPOD *node;

		if((node = receiver.getNode<NodePointStreamFilterPOD>()) != NULL)
		{
			node->getIndexStream()->addPassPointIntensity(/*geomd,*/ intensity);
		}
	}

}


void PointStreamFilter::buildNormals(bool transform, float quality)
{
	// Not Implemented
}

void PointStreamFilter::deleteAll(void)
{
	spatialPartitionGraph.deleteAll();
}


void PointStreamFilter::getIndexStreams(std::vector<IndexStream *> &result)
{
	unsigned int					t;
	SpatialPartitioner::NodeHSet	set;

    SpatialPartitioner::NodeH rootNode = spatialPartitionGraph.getRootNode();
	spatialPartitionGraph.listDepthFirst(set, rootNode, NodeTypePointStreamFilterPOD);

	for(t = 0; t < set.size(); t++)
	{
		SpatialPartitioner::NodeH	node = set[t];

		NodePointStreamFilterPOD *pod = dynamic_cast<NodePointStreamFilterPOD *>(node.getNode());

		result.push_back(pod->getIndexStream());
	}

}


}
