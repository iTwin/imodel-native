// IClashNode.h
#ifndef ICLASHNODE_AFFA0BAD_9619_4678_9438_5BD671057CE5_H
#define ICLASHNODE_AFFA0BAD_9619_4678_9438_5BD671057CE5_H


#include "VortexObjects_import.h"


namespace vortex
{	
	//------------------------------------------------------------------------
	// IClashNode
	//------------------------------------------------------------------------
	class PTVOBJECT_API IClashNode
	{
	protected:		
		virtual IClashNode*	_getLeft() = 0;
		virtual IClashNode*	_getRight() = 0;
		virtual PTbool		_isLeaf() = 0;
		virtual PTvoid		_getBounds(PTfloat* extents3, PTdouble* center3, PTfloat* xAxis3, PTfloat* yAxis3, PTfloat* zAxis3) = 0;

	public:		
		//------------------------------------------------------------------------
		// Get the left child node of this node
		// @return	a valid IClashNode pointer to the left child node of this node
		//			or PT_NULL if this node has no left child node.
		//------------------------------------------------------------------------
		IClashNode*	getLeft();

		//------------------------------------------------------------------------
		// Get the right child node of this node
		// @return	a valid IClashNode pointer to the right child node of this node
		//			or PT_NULL if this node has no left child node.	
		//------------------------------------------------------------------------
		IClashNode*	getRight();

		//------------------------------------------------------------------------
		// Check if this node is a leaf node
		// @return	true if this node is a leaf node (i.e. it has no child nodes),
		//			false if it is not a leaf node.
		//------------------------------------------------------------------------
		PTbool		isLeaf();

		//------------------------------------------------------------------------
		// Get the non axis-aligned bounds of this node in local coordinates, 
		// with an x,y and z axis that gives the orientation of the box.
		// @param extents3	An array of 3 float values to receive x,y,z extents
		// @param center3	An array of 3 float values to receive the center position
		// @param xAxis3	An array of 3 float values to receive the x axis of the 
		//					coordinate system for these bounds
		// @param yAxis3	An array of 3 float values to receive the y axis of the 
		//					coordinate system for these bounds
		// @param zAxis3	An array of 3 float values to receive the z axis of the 
		//					coordinate system for these bounds
		// @return
		//------------------------------------------------------------------------
		PTvoid		getBounds(PTfloat* extents3, PTdouble* center3, PTfloat* xAxis3, PTfloat* yAxis3, PTfloat* zAxis3);
	};	
}

#endif // ICLASHNODE_AFFA0BAD_9619_4678_9438_5BD671057CE5_H
