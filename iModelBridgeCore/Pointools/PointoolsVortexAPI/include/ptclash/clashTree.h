#pragma once
#include <pt/boundsTree.h>
#include <pt/ObjectRef.h>
#include <ptfs/FilePath.h>
#include <ptfs/iohelper.h>
#include <vortexobjects/IClashTree.h>


namespace pt
{

class ClashTree : public vortex::IClashTree
{
public:
	ClashTree( const pt::PersistentObjectRef &ref ) : m_ref(ref), m_tree(0)
	{
		m_maxLeafDepth = -1;
		m_minLeafDepth = -1;
		m_complete = true;
		m_numLeaves = 0;
	}
	
	ClashTree( const pt::PersistentObjectRef &ref, OrientedBoxBoundsTreed *tree, double minDim, double maxDim, int tarPtsPerLeaf )
		: m_ref(ref), m_tree( tree ), m_minDim(minDim), m_maxDim(maxDim), m_targetPtsPerLeaf(tarPtsPerLeaf),
		m_complete( true )
	{}
	
	virtual ~ClashTree();

	bool					saveToFile( const ptds::FilePath &file, const mmatrix4d &currentTransform );
	bool					loadFromFile( const ptds::FilePath &file, mmatrix4d &currentTransform );

	OrientedBoxBoundsTreed			*tree()			{ return m_tree; }
	const OrientedBoxBoundsTreed	*tree() const	{ return m_tree; }
	
	double					minLeafDim() const		{ return m_minDim; }
	double					maxLeafDim() const		{ return m_maxDim; }
	int						numLeaves() const		{ return m_numLeaves; }

	int						targetPtsPerLeaf() const { return m_targetPtsPerLeaf; }

#ifdef HAVE_OPENGL
    // for diagnostics
	int						drawLeaves() const;
	int						drawNodes(int level) const;
	int						drawTree() const;
#endif

	int						maxLeafDepth();
	int						minLeafDepth();
	
	bool					isComplete() const		{ return m_complete; }

	bool					computeLeafBounds( pt::OBBoxd &box ) const;

	void					orphanTree();

	//------------------------------------------------------------------------
	// IClashTree
	//------------------------------------------------------------------------
	virtual vortex::IClashNode*		_getRoot(PTvoid);	
	virtual PTint					_getNumLeaves(PTvoid);
	virtual PTres					_getLeafBounds(PTfloat* extents, PTdouble* center, PTfloat* xAxis, PTfloat* yAxis, PTfloat* zAxis);


protected:

#ifdef HAVE_OPENGL
	void					drawBox( const OBBoxd &box ) const;
#endif

	OrientedBoxBoundsTreed::Node *readNode(ptds::ReadBlock &reader, OrientedBoxBoundsTreed::Node *parent=0);

	OrientedBoxBoundsTreed	*m_tree;
	
	double					m_minDim;
	double					m_maxDim;
	int						m_targetPtsPerLeaf;
	int						m_minLeafDepth;
	int						m_maxLeafDepth;
	bool					m_complete;
	int						m_numLeaves;

	const pt::PersistentObjectRef& m_ref;
};

}
