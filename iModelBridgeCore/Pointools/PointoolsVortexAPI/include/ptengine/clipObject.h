//----------------------------------------------------------------------------
//
// clipObject.h
//
// Copyright (c) 2014 Bentley Systems, Incorporated. All rights reserved.
//
//----------------------------------------------------------------------------
#pragma once

#include <pt/os.h>
#include <pt/typedefs.h>
#include <ptapi/PointoolsVortexAPI.h>

namespace pointsengine
{

class ClipObject
{
public:
	ClipObject();
	virtual ~ClipObject() { ; }

	void enable(bool on) { m_enabled = on; }	
	bool enabled() { return m_enabled; }

	PTuint id() { return m_id; }

protected:
	ClipObject(PTuint id) : m_id(id), m_enabled(false) { ; }	

private:
	PTuint	m_id;
	bool	m_enabled;

	static PTuint s_clipObjectsCount;

};

}
