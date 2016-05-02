#include "PointoolsVortexAPIInternal.h"
// IClashTree.cpp

#include <vortexobjects/IClashTree.h>


using namespace vortex;


//---------------------------------------------------------------------------
// Public NVI
//---------------------------------------------------------------------------
IClashNode* IClashTree::getRoot(PTvoid)
{
	return _getRoot();
}

PTint IClashTree::getNumLeaves(PTvoid)
{
	return _getNumLeaves();
}

PTres IClashTree::getLeafBounds(PTfloat* extents, PTdouble* center, PTfloat* xAxis, PTfloat* yAxis, PTfloat* zAxis)
{
	return _getLeafBounds(extents, center, xAxis, yAxis, zAxis);
}
