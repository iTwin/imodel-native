#include "PointoolsVortexAPIInternal.h"
// IClashTree.cpp

#include <Vortex/IClashTree.h>

using namespace vortex;


//---------------------------------------------------------------------------
// Public NVI
//---------------------------------------------------------------------------
IClashNode* IClashTree::getRoot()
{
	return _getRoot();
}

PTint IClashTree::getNumLeaves()
{
	return _getNumLeaves();
}

PTres IClashTree::getLeafBounds(PTfloat* extents, PTdouble* center, PTfloat* xAxis, PTfloat* yAxis, PTfloat* zAxis)
{
	return _getLeafBounds(extents, center, xAxis, yAxis, zAxis);
}
