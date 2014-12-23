//----------------------------------------------------------------------------
//
// isolationFilter.cpp
//
// Copyright (c) 2014 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------

#include <ptedit/isolationFilter.h>
#include <ptengine/clipManager.h>

using namespace ptedit;


SelectionResult IsolationFilterClip::intersect(const pt::BoundingBoxD &box)
{
	return pointsengine::ClipManager::instance().intersect(box);
}

bool IsolationFilterClip::inside(int thread, const pt::vector3d &pnt)
{
	return pointsengine::ClipManager::instance().inside(thread, pnt);
}
