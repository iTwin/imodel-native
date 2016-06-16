//----------------------------------------------------------------------------
//
// isolationFilter.h
//
// Copyright (c) 2014 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------

#pragma once

#include <pt/boundingbox.h>
#include <ptedit/edit.h>

#include <math/matrix_math.h>
#include <fastdelegate/fastdelegate.h>


namespace ptedit
{

/** Structure for storing the current isolation filter's intersect() and inside() functions.
 The IsolationFilterManager makes sure that these are up to date. Users should call
 IsolationFilterManager::Get
 */
struct IsolationFilter
{
	typedef fastdelegate::FastDelegate1<const pt::BoundingBoxD&, SelectionResult> IntersectCallback;	
	typedef fastdelegate::FastDelegate2<int, const pt::vector3d&, bool> InsideCallback;	

	IntersectCallback intersect;
	InsideCallback inside;
};

//----------------------------------------------------------------------------
// Isolation Filters
// All isolation filters should have an intersect and an inside function
// defined with the same args/return types as IsolationFilterNone.
//----------------------------------------------------------------------------

/** Null isolation filter.
 */
struct IsolationFilterNone
{
public:
	/** Check intersection of the passed bounding box with the bounding box of this filter.
	 @param box			A bounding box to be checked against the bounds of this filter.
	 @return			Always returns FullyInside as this is a null filter.
	 */
	SelectionResult intersect(const pt::BoundingBoxD &box)
	{
		return FullyInside;
	}

	/** Check if the passed point is inside this filter box.
	 @param thread	The thread number where 0 <= thread < EDT_MAX_THREADS.
	 @param pnt		The points to check against this filter box.
	 @return		Always returns true as this is a null filter.
	 */
	bool inside(int thread, const pt::vector3d &pnt)
	{
		return true;
	}
};

/** Clip isolation filter, refer all tests to the ClipManager.
 */
struct IsolationFilterClip
{
public:
	/** Check intersection of the passed bounding box with the current clip manager.
	 @param box			A bounding box to be checked against the bounds of this filter.
	 @return			FullyInside, PartiallyInside or FullyOutside
	 */
	SelectionResult intersect(const pt::BoundingBoxD &box);

	/** Check if the passed point is inside the current clip objects.
	 @param thread	The thread number where 0 <= thread < EDT_MAX_THREADS.
	 @param pnt		The points to check against the current clip objects.
	 @return		true if inside, false otherwise
	 */
	bool inside(int thread, const pt::vector3d &pnt);
};

///** Isolation filter in the shape of a box. Anything outside the box will be filtered out.
// */
//struct IsolationFilterBox
//{
//public:
//	IsolationFilterBox()
//	{
//		m_bb.makeEmpty();
//		m_angles.zero();
//
//		m_centeredbb.makeEmpty();
//		m_mat.identity();
//	}
//
//	/** Check intersection of the passed bounding box with the bounding box of this filter.
//	 @param box			A bounding box to be checked against the bounds of this filter.
//	 @return
//	 FullyInside		The passed box is fully inside the filter's box
//	 PartiallyInside	The passed box partially intersects the filter's box
//	 FullyOutside		The passed box is completely outside the filter's box
//	 */
//	SelectionResult intersect(const pt::BoundingBoxD &box)
//	{
//		SelectionResult res = FullyOutside;
//
//		// Optimized version for non-rotated isolation filter
//		if (m_angles.is_zero())
//		{
//			if (m_bb.contains(&box))
//				res = FullyInside;
//			else if (m_bb.intersects(&box))
//				res = PartiallyInside;
//		}
//		else
//		{
//			pt::vector3d v, v1;
//			pt::BoundingBoxD env;
//			env.clear();
//
//			// Transform each corner of the box to test so it is in the same
//			// coordinate space as the rotated centered box before testing
//			for (int i = 0; i < 8; i++)
//			{
//				box.getExtrema(i, v);
//				m_mat.vec3_multiply_mat4d(v, v1);
//				env.expand(v1);
//			}
//			if (m_centeredbb.contains(&env)) 
//				res = FullyInside;
//			if (m_centeredbb.intersects(&env)) 
//				res = PartiallyInside;
//		}
//
//		return res;
//	}
//
//	/** Check if the passed point is inside this filter box.
//	 @param thread	The thread number where 0 <= thread < EDT_MAX_THREADS.
//	 @param pnt		The points to check against this filter box.
//	 @return
//	 true	The point is inside the bounds of this filter box.
//	 false	The point is not inside this filter box.
//	 */
//	bool inside(int thread, const pt::vector3d &pnt)
//	{
//		bool res[EDT_MAX_THREADS];
//		pt::vector3d pntf[EDT_MAX_THREADS];
//		pntf[thread].set(pnt.x, pnt.y, pnt.z);
//
//		if (m_angles.is_zero())
//		{
//			res[thread] = m_bb.inBounds(pntf[thread]);
//		}
//		else
//		{
//			// Transform the point to test so it is in the same
//			// coordinate space as the rotated centered box before testing
//			pt::vector3d v1[EDT_MAX_THREADS];
//			m_mat.vec3_multiply_mat4d(pntf[thread], v1[thread]);
//			res[thread] = m_centeredbb.inBounds(v1[thread]);
//		}
//
//		return res[thread];
//	}
//
//	void SetBounds(const pt::BoundingBoxD& bb, const pt::vector3& angles) 
//	{ 
//		m_bb = bb; 
//		m_angles = angles;
//
//		// store a centered version of the bounding box and a matrix for
//		// transforming objects to the coordinate space of the centered box.
//		m_centeredbb = m_bb;
//		m_centeredbb.translateBy(-bb.center());
//		m_mat = mmatrix4d::rotation(DegToRad(m_angles.x), DegToRad(m_angles.y), DegToRad(m_angles.z));
//		m_mat.translate(pt::vector4d(m_bb.center()));
//		m_mat.invert();
//	};
//
//private:
//	pt::BoundingBoxD m_bb;
//	pt::vector3 m_angles;
//
//	pt::BoundingBoxD m_centeredbb;
//	mmatrix4d	m_mat;
//};

};
