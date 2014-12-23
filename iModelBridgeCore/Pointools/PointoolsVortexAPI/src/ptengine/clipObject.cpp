//----------------------------------------------------------------------------
//
// clipObject.cpp
//
// Copyright (c) 2014 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#include <ptengine/clipObject.h>
#include <ptcloud2/pointcloud.h>


namespace pointsengine
{

PTuint ClipObject::s_clipObjectsCount = 0;

ClipObject::ClipObject() : 
m_id(s_clipObjectsCount++),
m_enabled(false)
{  

}

}
