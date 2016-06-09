#include "PointoolsVortexAPIInternal.h"
// clashTree.cpp

#include <ptapi/PointoolsVortexAPI.h>
#include <ptclash/clashTree.h>
#include <ptfs/filepath.h>
#include <ptcloud2/pointcloud.h>
#include <ptcloud2/scene.h>


//-----------------------------------------------------------------------------
// EXTERNS
//-----------------------------------------------------------------------------
extern pcloud::Scene*			sceneFromHandle(PThandle handle);
extern pcloud::PointCloud*		cloudFromHandle(PThandle cloud);


using namespace pt;

struct NodeWriter
{
	NodeWriter( ptds::WriteBlock &i ) : io(i) {}
	NodeWriter( ptds::WriteBlock &i, mmatrix4d &nodeTransform) : io(i), transform(nodeTransform) {}

	bool node( OrientedBoxBoundsTreed::Node *node,  OrientedBoxBoundsTreed::Node *parent )
	{
		if (!node)
			io.write( (int)-1 );
		else
		{
			node->writeNode( io, transform);
		}
		return true;
	}
	ptds::WriteBlock &	io;
	mmatrix4d			transform;
};
//-----------------------------------------------------------------------------
ClashTree::~ClashTree()
//-----------------------------------------------------------------------------
{
	if (m_tree)
		delete m_tree;
}
//-----------------------------------------------------------------------------
bool		ClashTree::saveToFile( const ptds::FilePath &file, const mmatrix4d &currentTransform )
//-----------------------------------------------------------------------------
{
	if (!m_tree) return false;

	ptds::DataSourcePtr h = ptds::dataSourceManager.openForWrite( &file );
	if (!h || !(h->validHandle())) 
		return false;

	{
		ptds::Tracker tracker(h);
		ptds::WriteBlock io(h, 1048576, &tracker);

		int version = 1;
		
		// header
		io.write(version);
		io.write((float) m_minDim);
		io.write((float) m_maxDim);
		io.write(m_targetPtsPerLeaf);

		mmatrix4d identityTransform = mmatrix4d::identity();
		io.write(identityTransform.data(), sizeof(double)*16);

		mmatrix4d currentTransformInv;

		currentTransformInv = currentTransform;
		currentTransformInv.invert();

		NodeWriter writer(io, currentTransformInv);
		m_tree->root()->visit( writer, true );
		
		io.close();	
	}

	m_minLeafDepth = m_tree->root()->findMinLeafDepth();
	m_maxLeafDepth = m_tree->root()->findMaxLeafDepth();
	m_numLeaves = m_tree->root()->countLeaves();

	ptds::dataSourceManager.close(h);

	return true;
}
//-----------------------------------------------------------------------------
OrientedBoxBoundsTreed::Node * ClashTree::readNode(ptds::ReadBlock &reader, OrientedBoxBoundsTreed::Node *parent)
//-----------------------------------------------------------------------------
{
	OBBoxd bounds;
	
	if (!parent)	// ie root
	{
		parent = new OrientedBoxBoundsTreed::Node( bounds, 0, 0 );
		parent->readNode(reader);	// read root node

		readNode(reader, parent);	// read L
		readNode(reader, parent); // read R

		return parent;	// final return
	}
	int flags = parent->readAndAddChildNode(reader, bounds);

	if (flags > 0)
	{
		if ((uint)flags & OrientedBoxBoundsTreed::Node::LeftNode)
		{
			readNode(reader, parent->left());	// read L
			readNode(reader, parent->left());	// read R
		}
		else if ((uint)flags & OrientedBoxBoundsTreed::Node::RightNode)
		{
			readNode(reader, parent->right());	// read L
			readNode(reader, parent->right());	// read R
		}
	}
	return 0;
}
//-----------------------------------------------------------------------------
bool		ClashTree::loadFromFile( const ptds::FilePath &file, mmatrix4d &currentTransform )
//-----------------------------------------------------------------------------
{
	ptds::DataSourcePtr h = ptds::dataSourceManager.openForRead( &file );
	if (!h || !(h->validHandle())) 
		return false;

	ptds::Tracker tracker(h);
	ptds::ReadBlock io(h, &tracker);

	int version=0;
	io.read(version);

	if (m_tree) delete m_tree;
	m_tree = 0;

	if (version == 1)
	{
		// header

		float minDim;
		float maxDim;

		io.read(minDim);
		io.read(maxDim);
		io.read(m_targetPtsPerLeaf);
		io.read(currentTransform.data(), 16 * sizeof(double));

		m_minDim = minDim;
		m_maxDim = maxDim;

		//nodes
		OrientedBoxBoundsTreed::Node *root = readNode( io );
		m_tree = new OrientedBoxBoundsTreed(root);

		m_minLeafDepth = m_tree->root()->findMinLeafDepth();
		m_maxLeafDepth = m_tree->root()->findMaxLeafDepth();
		m_numLeaves = m_tree->root()->countLeaves();
	}
	ptds::dataSourceManager.close( h );
	return m_tree ? true : false;
}

#ifdef HAVE_OPENGL
//-----------------------------------------------------------------------------
void ClashTree::drawBox( const OBBoxd &box ) const
//-----------------------------------------------------------------------------
{
	vector3d vertices[8];
	box.computeVertices( vertices );

	// simple wire-frame box
	glBegin(GL_LINE_STRIP);
		glVertex3dv( vertices[0] );
		glVertex3dv( vertices[1] );
		glVertex3dv( vertices[3] );
		glVertex3dv( vertices[2] );
		glVertex3dv( vertices[0] );
	glEnd();

	glBegin(GL_LINE_STRIP);
		glVertex3dv( vertices[4] );
		glVertex3dv( vertices[6] );
		glVertex3dv( vertices[7] );
		glVertex3dv( vertices[5] );
		glVertex3dv( vertices[4] );
	glEnd();

	glBegin(GL_LINES);
		glVertex3dv( vertices[3] );
		glVertex3dv( vertices[7] );

		glVertex3dv( vertices[2] );
		glVertex3dv( vertices[6] );

		glVertex3dv( vertices[0] );
		glVertex3dv( vertices[4] );

		glVertex3dv( vertices[1] );
		glVertex3dv( vertices[5] );
	glEnd(); 
}
//-----------------------------------------------------------------------------
int ClashTree::drawLeaves() const
//-----------------------------------------------------------------------------
{
	static int interation = 0;

	if (m_tree && m_tree->root())
	{
		std::vector< OBBoxd > boxes;
		m_tree->root()->collectLeaves(boxes);

		for (int i=0; i<boxes.size(); i++)
		{
			drawBox( boxes[i] );
		}
		return boxes.size();
	}
	return 0;
}
//-----------------------------------------------------------------------------
int ClashTree::drawTree() const
//-----------------------------------------------------------------------------
{
	if (m_tree && m_tree->root())
	{
		std::vector< OBBoxd > boxes;
		m_tree->root()->collectLeaves(boxes);

		for (int i=0; i<boxes.size(); i++)
		{	
			drawBox( boxes[i] );
		}
		return boxes.size();
	}
	return 0;
}
//-----------------------------------------------------------------------------
int ClashTree::drawNodes( int level ) const
//-----------------------------------------------------------------------------
{
	if (m_tree && m_tree->root())
	{
		std::vector< OBBoxd > boxes;
		m_tree->root()->collectNodes(boxes, level);

		for (int i=0; i<boxes.size(); i++)
		{	
			drawBox( boxes[i] );
		}
		return boxes.size();
	}
	return 0;
}
#endif

//-----------------------------------------------------------------------------
int	ClashTree::maxLeafDepth()
//-----------------------------------------------------------------------------
{
	if (m_minLeafDepth < 1)
	{
		m_maxLeafDepth = m_tree->root()->findMaxLeafDepth();
		m_minLeafDepth = m_tree->root()->findMinLeafDepth();
	}
	return m_maxLeafDepth;
}
//-----------------------------------------------------------------------------
int	ClashTree::minLeafDepth()
//-----------------------------------------------------------------------------
{
	if (m_minLeafDepth < 1)
	{
		m_maxLeafDepth = m_tree->root()->findMaxLeafDepth();
		m_minLeafDepth = m_tree->root()->findMinLeafDepth();
	}
	return m_minLeafDepth;
}
//-----------------------------------------------------------------------------
bool ClashTree::computeLeafBounds( pt::OBBoxd &box ) const
//-----------------------------------------------------------------------------
{
	// go through leaves and compute overall bounds
	if (m_tree && m_tree->root())
	{
		std::vector< OBBoxd > boxes;
		m_tree->root()->collectLeaves(boxes);

		if (boxes.size())
		{
			std::vector<vector3d> pts;

            for (size_t i = 0; i < boxes.size(); i++)
			{
				vector3d verts[8];
				boxes[i].computeVertices(verts);

				for (int k=0;k<8;k++) 
					pts.push_back(verts[k]);				
			}
			box = createFittingOBBd(pts);	
			
			return true;
		}
	}
	return false;
}
//-----------------------------------------------------------------------------
void	ClashTree::orphanTree()
//-----------------------------------------------------------------------------
{
	m_tree = 0;
}

//-----------------------------------------------------------------------------
// IClashTree
//-----------------------------------------------------------------------------
vortex::IClashNode* ClashTree::_getRoot()
{
	if (m_tree)
		return m_tree->root();

	return PT_NULL;
}

PTint ClashTree::_getNumLeaves()
{
	return numLeaves();
}

PTres ClashTree::_getLeafBounds(PTfloat* extents, PTdouble* center, PTfloat* xAxis, PTfloat* yAxis, PTfloat* zAxis)
{
	if (!extents || !center || !xAxis || !yAxis || !zAxis)
		return PTV_INVALID_PARAMETER;

	std::vector< OBBoxd > boxes;
	m_tree->root()->collectLeaves(boxes);

	// The arrays passed to this function should have been allocated
	// by the client as size 3*numLeaves() so make sure the number so make sure
	// we are using the same number of boxes here as expected
	if (boxes.size() != m_numLeaves)
		return PTV_UNKNOWN_ERROR;
				

	for (size_t i=0; i<boxes.size(); i++)
	{
		extents[i*3]     = static_cast<PTfloat>(boxes.at(i).extents().x);
		extents[(i*3)+1] = static_cast<PTfloat>( boxes.at(i).extents().y);
		extents[(i*3)+2] = static_cast<PTfloat>( boxes.at(i).extents().z);

		center[i*3]     =  static_cast<PTfloat>(boxes.at(i).center().x);
		center[(i*3)+1] =  static_cast<PTfloat>(boxes.at(i).center().y);
		center[(i*3)+2] =  static_cast<PTfloat>(boxes.at(i).center().z);

		xAxis[i*3]     =  static_cast<PTfloat>(boxes.at(i).axis(0).x);
		xAxis[(i*3)+1] =  static_cast<PTfloat>(boxes.at(i).axis(0).y);
		xAxis[(i*3)+2] =  static_cast<PTfloat>(boxes.at(i).axis(0).z);

		yAxis[i*3]     =  static_cast<PTfloat>(boxes.at(i).axis(1).x);
		yAxis[(i*3)+1] =  static_cast<PTfloat>(boxes.at(i).axis(1).y);
		yAxis[(i*3)+2] =  static_cast<PTfloat>(boxes.at(i).axis(1).z);

		zAxis[i*3]     =  static_cast<PTfloat>(boxes.at(i).axis(2).x);
		zAxis[(i*3)+1] =  static_cast<PTfloat>(boxes.at(i).axis(2).y);
		zAxis[(i*3)+2] =  static_cast<PTfloat>(boxes.at(i).axis(2).z);
	}

	return PTV_NOT_IMPLEMENTED_IN_VERSION;
}
