#pragma once

#include <ptedit/editSelect.h>


namespace ptedit
{
	class BoxSelect : public EditNodeDef
	{
	protected:

		pt::BoundingBoxD box;

	public: 

		BoxSelect() : EditNodeDef("Box")
		{
			box.makeEmpty();
		}

		DECLARE_EDIT_NODE( "Box", "Box Sel", 0, EditNodeMultithread | EditNodePostConsolidateVis );

		SELECTION_FILTER_DETAIL

		void set(const pt::vector3d &lower, const pt::vector3d &upper)
		{
			box.set(lower, upper);
		}

		bool apply() 
		{ 
			SelectionFilter<BoxSelect> f(*this);
			return f.processFilter(); 
		}

		void draw();

		bool writeState( pt::datatree::Branch *b) const;
		bool readState(const pt::datatree::Branch *);

		/* functions used in selection template */ 

		static SelectionResult intersect(const BoxSelect &boxSelect, const pt::BoundingBoxD &b)
		{

			if (boxSelect.contains(b))
				return FullyInside;

			if (boxSelect.intersects(b))
				return PartiallyInside;

			return FullyOutside;
		}

		inline static bool inside(int thread, const BoxSelect &boxSelect, const pt::vector3d &pnt)
		{
			return boxSelect.box.inBounds(pnt);
		}

		inline bool contains(const pt::BoundingBoxD &b) const
		{
			return box.contains(&b);			
		}

		inline bool intersects(const pt::BoundingBoxD &b) const
		{
			return box.intersects(&b);
		}

	};



	class OrientedBoxSelect : public EditNodeDef
	{
	protected:

		pt::BoundingBoxD box;

		mvector4d		position;
		mvector4d		uAxis;
		mvector4d		vAxis;
		mvector4d		wAxis;

		mmatrix4d		transformWorldToLocal;

	public: 

		OrientedBoxSelect() : EditNodeDef("OrientedBox")
		{
			box.makeEmpty();
		}

		DECLARE_EDIT_NODE( "OrientedBox", "OrientedBox Sel", 0, EditNodeMultithread | EditNodePostConsolidateVis );

		SELECTION_FILTER_DETAIL

		void set(const pt::vector3d &lower, const pt::vector3d &upper)
		{
			box.set(lower, upper);
		}

		bool apply() 
		{ 
			SelectionFilter<OrientedBoxSelect> f(*this);
			return f.processFilter(); 
		}

		void draw();

		bool writeState( pt::datatree::Branch *b) const;
		bool readState(const pt::datatree::Branch *);

		/* functions used in selection template */ 

		static SelectionResult intersect(const OrientedBoxSelect &boxSelect, const pt::BoundingBoxD &box)
		{

															// Create box vertices in world coordinates
			mvector4d	c1(box.lower(0), box.lower(1), box.lower(2));
			mvector4d	c2(box.upper(0), box.lower(1), box.lower(2));
			mvector4d	c3(box.upper(0), box.upper(1), box.lower(2));
			mvector4d	c4(box.lower(0), box.upper(1), box.lower(2));

			mvector4d	c5(box.lower(0), box.lower(1), box.upper(2));
			mvector4d	c6(box.upper(0), box.lower(1), box.upper(2));
			mvector4d	c7(box.upper(0), box.upper(1), box.upper(2));
			mvector4d	c8(box.lower(0), box.upper(1), box.upper(2));

			mvector4d	tc1, tc2, tc3, tc4;
			mvector4d	tc5, tc6, tc7, tc8;

															// Transform box vertices into OOBB's local coordinate system
			c1.vec3_multiply_mat4(boxSelect.transformWorldToLocal, tc1);
			c2.vec3_multiply_mat4(boxSelect.transformWorldToLocal, tc2);
			c3.vec3_multiply_mat4(boxSelect.transformWorldToLocal, tc3);
			c4.vec3_multiply_mat4(boxSelect.transformWorldToLocal, tc4);
			c5.vec3_multiply_mat4(boxSelect.transformWorldToLocal, tc5);
			c6.vec3_multiply_mat4(boxSelect.transformWorldToLocal, tc6);
			c7.vec3_multiply_mat4(boxSelect.transformWorldToLocal, tc7);
			c8.vec3_multiply_mat4(boxSelect.transformWorldToLocal, tc8);

			pt::BoundingBoxD approxBox;
															// Calculate extents of transformed box to form an approximate AABB
			approxBox.expand(tc1.data());
			approxBox.expand(tc2.data());
			approxBox.expand(tc3.data());
			approxBox.expand(tc4.data());
			approxBox.expand(tc5.data());
			approxBox.expand(tc6.data());
			approxBox.expand(tc7.data());
			approxBox.expand(tc8.data());
															// Test whether OOBB contains approximated AABB
			if(boxSelect.box.contains(&approxBox))
				return FullyInside;
															// If test box contains this oriented bounding box, it is partial
															// and must therefore be tested further
			if(approxBox.contains(&boxSelect.box))
				return PartiallyInside;

			if(boxSelect.box.intersects(&approxBox))
				return PartiallyInside;
															// Box must be outside
			return FullyOutside;
		}

		inline static bool inside(int thread, const OrientedBoxSelect &boxSelect, const pt::vector3d &pnt)
		{
			mvector4d	p(pnt[0], pnt[1], pnt[2], 1);
			mvector4d	localP;
															// Transform point into local coordinate system
			p.vec3_multiply_mat4(boxSelect.transformWorldToLocal, localP);
															// Test
			return boxSelect.box.inBounds(localP.data());
		}

		void setTransform(const pt::vector3d &initPosition, const pt::vector3d &initUAxis, const pt::vector3d &initVAxis)
		{
			// Copy position
			position(0, 0)	= initPosition[0];
			position(1, 0)	= initPosition[1];
			position(2, 0)	= initPosition[2];
			position(3, 0)	= 0;
			// Copy local coordinate system U and V vectors (X and Y)
			uAxis(0, 0)		= initUAxis[0];
			uAxis(1, 0)		= initUAxis[1];
			uAxis(2, 0)		= initUAxis[2];
			uAxis(3, 0)		= 0;

			vAxis(0, 0)		= initVAxis[0];
			vAxis(1, 0)		= initVAxis[1];
			vAxis(2, 0)		= initVAxis[2];
			vAxis(3, 0)		= 0;
			// Make sure input vectors are normalized
			uAxis.normalize();
			vAxis.normalize();
			// Calculate W axis (Z) as cross product of U (X) and V (Y)
			mvector4d	wAxis;
			wAxis = uAxis;
			wAxis.cross(vAxis);
			wAxis.normalize();

			// Calculate inverse translation matrix
			mvector4d	invTransVec = -position;
			invTransVec(3, 0) = 1;

			mmatrix4d	invTranslation;
			mmatrix4d	invRotation;
			// Build inverse translation matrix
			invTranslation = mmatrix4d::identity();
			invTranslation = mmatrix4d::translation(invTransVec.data());

			// Calculate inverse rotation matrix
			invRotation = mmatrix4d::identity();
			invRotation.setVector(0, uAxis.data());
			invRotation.setVector(1, vAxis.data());
			invRotation.setVector(2, wAxis.data());
			// Calculate world to local transformation matrix
			// as invTranslation * invRotation
			transformWorldToLocal = invTranslation >> invRotation;
		}

	};

}

