/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <pt/ViewParams.h>
#include <ptcloud2/voxel.h>

namespace ProjArea
{

struct AABB_Approximator
{
	static float pixArea( const pcloud::Node *v, const pt::ViewParams &vs );
	static float planeArea( const pcloud::Node *v, const pt::ViewParams &vs );
};
struct Scanline_Approximator
{
	static float pixArea( const pcloud::Node *v, const pt::ViewParams &vs );
	static float planeArea( const pcloud::Node *v, const pt::ViewParams &vs );
};
struct ConvexHull_Approximator
{
	static float pixArea( const pcloud::Node *v, const pt::ViewParams &vs );
	static float planeArea( const pcloud::Node *v, const pt::ViewParams &vs );
};

}
