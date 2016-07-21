#include "PointoolsVortexAPIInternal.h"
// IClashTree.cpp

#include <Vortex/IClashNode.h>


using namespace vortex;


//---------------------------------------------------------------------------
// Public NVI
//---------------------------------------------------------------------------
IClashNode*	IClashNode::getLeft()
{
	return _getLeft();
}

IClashNode*	IClashNode::getRight()
{
	return _getRight();
}

PTbool IClashNode::isLeaf()
{
	return _isLeaf();
}

PTvoid IClashNode::getBounds(PTfloat* extents3, PTdouble* center3, PTfloat* xAxis3, PTfloat* yAxis3, PTfloat* zAxis3)
{
	return _getBounds(extents3, center3, xAxis3, yAxis3, zAxis3);
}
