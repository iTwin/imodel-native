/*--------------------------------------------------------------------------*/ 
/*  Pageable.h																*/ 
/*	Geometry base class definition											*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Jan 2004 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef COMMONCLASSES_PAGEABLE_DEFINITION
#define COMMONCLASSES_PAGEABLE_DEFINITION

#include <pt/classes.h>

namespace pt
{
template <class BaseClass>
class Pageable : public BaseClass
{
public:
	Pageable() 
		: m_lod(MIN_LOD), 
		m_request_lod(MIN_LOD), 
		m_frames_since_request(0), 
		m_filepointer(-1) {};

	/*frames*/ 
	void used()		{ m_frames_since_request = 0; }
	void unused()	{ m_frames_since_request++; }
	int framesSinceRequest() const { return m_frames_since_request; }

	/*request*/ 
	inline void request(float lod)	{ m_request_lod = (255 * lod); }
	inline float request() const	{ return ((float)m_request_lod) / 255.0f; }
	inline float lod() const		{ return ((float)m_lod) / 255.0f; }
	inline const unsigned char &lodAsByte() const		{ return m_lod; }
	inline const unsigned char &requestByte() const	{ return m_request_lod; }

	void	filePointer(int64_t pos)	{ m_filepointer = pos; }
	int64_t filePointer() const			{ return m_filepointer; }

	int		fileIndex() const			{ return (int)m_fileindex; }
	void	fileIndex(int filei)		{ m_fileindex = (unsigned char)filei; }


protected:
	unsigned char m_frames_since_request;
	unsigned char m_request_lod;
	unsigned char m_lod;
	unsigned char m_fileindex;

	int64_t	m_filepointer;
};
}
#endif
