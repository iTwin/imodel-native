// IClashTree.h
#ifndef ICLASHTREE_85A8A7EF_DFFE_4EDA_87B7_51EAC1663006_H
#define ICLASHTREE_85A8A7EF_DFFE_4EDA_87B7_51EAC1663006_H


#include "VortexObjects_import.h"


namespace vortex
{
	class IClashNode;

	//------------------------------------------------------------------------
	// IClashTree
	//------------------------------------------------------------------------
	class PTVOBJECT_API IClashTree
	{
	protected:
		virtual IClashNode* _getRoot() = 0;
		
		virtual PTint		_getNumLeaves() = 0;

		virtual PTres		_getLeafBounds(PTfloat* extents, PTdouble* center, PTfloat* xAxis, PTfloat* yAxis, PTfloat* zAxis) = 0;

	public:
		//--------------------------------------------------------------------
		// Get the root node of this IClashTree object		
		// @return		A pointer to the clash tree root node is returned here, 
		//				or PT_NULL is returned if no root node is found		
		//--------------------------------------------------------------------
		IClashNode* getRoot();

		//--------------------------------------------------------------------
		// Get the number of leaf nodes that this clash tree has
		// @return		The number of leaf nodes that this clash tree has
		//--------------------------------------------------------------------
		PTint		getNumLeaves();

		//--------------------------------------------------------------------
		// Get the non-axis aligned leaf node bounds of leaf nodes in this 
		// clash tree. These bounds are returned in arrays of floats that must 
		// be allocated as size PTfloat * 3 times the number of leaves 
		// (get the number of leaves by calling getNumLeaves()).
		//
		// For example: PTfloat* extents = new PTFloat[3*tree->getNumLeaves()]
		//
		// The caller is responsible for allocating these arrays and for 
		// deleting them.
		//
		// All values are returned in groups of 3, e.g. the x,y,z extents for 
		// the first leaf node will be returned in extents[0],extents[1],extents[2],
		// and the extents for the 2nd leaf node will be returned in 
		// extents[3],extents[4],extents[5] etc. The same follows for the other
		// arrays.
		//
		// @param extents	an array of PTfloat of size 3 * the number of leaves,
		//					the extents of each node are returned here
		// @param center	an array of PTfloat of size 3 * the number of leaves,
		//					the center point of each node is returned here
		// @param xAxis		an array of PTfloat of size 3 * the number of leaves,
		//					the normalized x axis vector for each node is returned
		//					here
		// @param yAxis		an array of PTfloat of size 3 * the number of leaves,
		//					the normalized y axis vector for each node is returned
		//					here
		// @param zAxis		an array of PTfloat of size 3 * the number of leaves,
		//					the normalized z axis vector for each node is returned
		//					here
		// @return		PTV_SUCCESS, leaf node details are returned,
		//				PTV_INVALID_PARAMETER one of the passed PTfloat arrays
		//				is NULL,
		//				PTV_UNKNOWN_ERROR if an error occurs getting the bounds
		//				of the leaf nodes internally
		//--------------------------------------------------------------------
		PTres		getLeafBounds(PTfloat* extents, PTdouble* center, PTfloat* xAxis, PTfloat* yAxis, PTfloat* zAxis);
	};
}

#endif // ICLASHTREE_85A8A7EF_DFFE_4EDA_87B7_51EAC1663006_H
