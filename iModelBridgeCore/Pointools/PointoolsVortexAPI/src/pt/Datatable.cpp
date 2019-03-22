/*--------------------------------------------------------------------------*/ 
/*  Datatable.cpp															*/ 
/*	Datatable base class definition	- run time struct definition			*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 9 Jan 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#include "PointoolsVortexAPIInternal.h"
#include <pt/datatable.h>
using namespace pt;

static char _iobuffer[128];

DataTable::DataTable()
{
}
DataTable::~DataTable()
{
	try
	{
	clear();
	}
	catch(...){}
}
void DataTable::addi(const DTID &id, int val)				{ if (!set(id, val)) integers.insert(INTMAP::value_type(id, val));	}
void DataTable::addf(const DTID &id, float val)				{ if (!set(id, val)) floats.insert(FLOATMAP::value_type(id, val));		}
void DataTable::addb(const DTID &id, bool val)				{ if (!set(id, val)) bools.insert(BOOLMAP::value_type(id, val));		}
void DataTable::addv(const DTID &id, const vector3 &val)	{ if (!set(id, val)) vectors.insert(VEC3MAP::value_type(id, val)); }
void DataTable::adds(const DTID &id, const char* val)		
{
	/*needs local storage - allocate string here*/ 
	if (!set(id, val)) 
	{
		strings.insert(STRINGMAP::value_type(id, pt::String(val)));	
	}
}
//
// Copying will not copy pointers!!!! never pass a datatable by value
// this should really be a "copy" method
//
DataTable &DataTable::operator = (const DataTable &dt)
{
	if (&dt == this) return *this;

	clear();

	/*copy data and structure*/ 
	INTMAP::const_iterator it_i= dt.integers.begin();
	while (it_i != dt.integers.end()) {	addi(it_i->first, it_i->second); ++it_i; }

	FLOATMAP::const_iterator it_f= dt.floats.begin();
	while (it_f != dt.floats.end()) {	addf(it_f->first, it_f->second); ++it_f; }

	BOOLMAP::const_iterator it_b= dt.bools.begin();
	while (it_b != dt.bools.end()) {	addb(it_b->first, it_b->second); ++it_b; }

	VEC3MAP::const_iterator it_v= dt.vectors.begin();
	while (it_v != dt.vectors.end()) {	addv(it_v->first, it_v->second); ++it_v; }

	STRINGMAP::const_iterator it_s= dt.strings.begin();
	while (it_s != dt.strings.end()) {	adds(it_s->first, it_s->second.c_str()); ++it_s; }

	return *this;
}

void DataTable::addP(const DTID &id, int *ptr)	    { if (!set(id, *ptr)) pintegers.insert(PINTMAP::value_type(id, ptr));	}
void DataTable::addP(const DTID &id, float *ptr)	{ if (!set(id, *ptr)) pfloats.insert(PFLOATMAP::value_type(id, ptr));	}
void DataTable::addP(const DTID &id, bool *ptr)		{ if (!set(id, *ptr)) pbools.insert(PBOOLMAP::value_type(id, ptr));		}
void DataTable::addP(const DTID &id, vector3 *ptr)	{ if (!set(id, *ptr)) pvectors.insert(PVEC3MAP::value_type(id, ptr));	}
void DataTable::addP(const DTID &id, String *ptr)	{ if (!set(id, ptr->c_str())) pstrings.insert(PSTRINGMAP::value_type(id, ptr));	}

void DataTable::clear()
{
	integers.clear();
	floats.clear();
	bools.clear();
	strings.clear();
	vectors.clear();

	pintegers.clear();
	pfloats.clear();
	pbools.clear();
	pstrings.clear();
	pvectors.clear();	
}
	
int DataTable::size()
{
	return 	static_cast<int>(integers.size() + floats.size() +  bools.size() +  strings.size() +  vectors.size() +
			                 pintegers.size() + 	pfloats.size() + pbools.size() + pstrings.size() + pvectors.size());
}
TYPE DataTable::type(const DTID &id)
{
	if (hasi(id)) return INTTYPE;
	if (hasf(id)) return FLOATTYPE;
	if (hasb(id)) return BOOLTYPE;
	if (hasv(id)) return VECTOR3TYPE;
	if (hass(id)) return STRINGTYPE;
	return NOTYPE;
}

/*mutation*/ 
bool DataTable::set(const DTID &id, const bool &val)
{
	BOOLMAP::iterator i = bools.find(id);	if (i != bools.end()) { i->second = val; return true; }
	PBOOLMAP::iterator j = pbools.find(id);	if (j != pbools.end()) { *j->second = val; return true; }
	return false;
}
bool DataTable::set(const DTID &id, const float &val)
{
	FLOATMAP::iterator i = floats.find(id);	if (i != floats.end()) { i->second = val; return true; }
	PFLOATMAP::iterator j = pfloats.find(id);	if (j != pfloats.end()) { *j->second = val; return true; }
	return false;
}
bool DataTable::set(const DTID &id, const int &val)
{
	INTMAP::iterator i = integers.find(id);		if (i != integers.end()) { i->second = val; return true; }
	PINTMAP::iterator j = pintegers.find(id);		if (j != pintegers.end()) { *j->second = val; return true; }
	return false;
}
bool DataTable::set(const DTID &id, const char *s)
{
	STRINGMAP::iterator i = strings.find(id); if (i != strings.end()) { i->second = s; return true; }
	PSTRINGMAP::iterator j = pstrings.find(id); if (j != pstrings.end()) { *j->second = s; return true; }
	return false;
}
bool DataTable::set(const DTID &id, const vector3 &val)
{
	VEC3MAP::iterator i = vectors.find(id);	if (i != vectors.end()) { i->second = val; return true; }
	PVEC3MAP::iterator j = pvectors.find(id);if (j != pvectors.end()) { *j->second = val; return true; }
	return false;
}

/*exists*/ 
bool DataTable::hasb(const DTID &id)
{
	BOOLMAP::iterator i = bools.find(id);	if (i != bools.end()) return true; 
	PBOOLMAP::iterator j = pbools.find(id);	if (j != pbools.end())	return true; 
	return false;
}
bool DataTable::hasf(const DTID &id)
{
	FLOATMAP::iterator i = floats.find(id);	if (i != floats.end())	 return true; 
	PFLOATMAP::iterator j = pfloats.find(id);if (j != pfloats.end()) return true; 
	return false;
}
bool DataTable::hasi(const DTID &id)
{
	INTMAP::iterator i = integers.find(id);		if (i != integers.end()) return true; 
	PINTMAP::iterator j = pintegers.find(id);	if (j != pintegers.end())return true; 
	return false;
}
bool DataTable::hass(const DTID &id)
{
	STRINGMAP::iterator i = strings.find(id);	if (i != strings.end())	 return true; 
	PSTRINGMAP::iterator j = pstrings.find(id);	if (j != pstrings.end()) return true; 
	return false;
}
bool DataTable::hasv(const DTID &id)
{
	VEC3MAP::iterator i = vectors.find(id);		if (i != vectors.end())	 return true; 
	PVEC3MAP::iterator j = pvectors.find(id);	if (j != pvectors.end()) return true; 
	return false;
}
/*access*/ 
bool DataTable::get(const DTID &id, bool &val)
{
	BOOLMAP::iterator i = bools.find(id);	if (i != bools.end())	{ val = i->second; return true; }
	PBOOLMAP::iterator j = pbools.find(id);	if (j != pbools.end())	{ val = *j->second; return true; }
	return false;
}
bool DataTable::get(const DTID &id, float &val)
{
	FLOATMAP::iterator i = floats.find(id);	if (i != floats.end())	{ val = i->second; return true; }
	PFLOATMAP::iterator j = pfloats.find(id);if (j != pfloats.end()) { val = *j->second; return true; }
	return false;
}
bool DataTable::get(const DTID &id, int &val)
{
	INTMAP::iterator i = integers.find(id);		if (i != integers.end())	{ val = i->second; return true; }
	PINTMAP::iterator j = pintegers.find(id);	if (j != pintegers.end())	{ val = *j->second; return true; }
	return false;
}
bool DataTable::get(const DTID &id, String &buff)
{
	STRINGMAP::iterator i = strings.find(id);	if (i != strings.end())		{ buff = i->second; return true; }
	PSTRINGMAP::iterator j = pstrings.find(id);	if (j != pstrings.end())	{ buff = *j->second; return true; }
	return false;
}
bool DataTable::get(const DTID &id, vector3 &val)
{
	VEC3MAP::iterator i = vectors.find(id);		if (i != vectors.end())		{ val = i->second; return true; }
	PVEC3MAP::iterator j = pvectors.find(id);	if (j != pvectors.end())	{ val = *j->second; return true; }
	return false;
}
//serialisation
//id, type, value tuples

//== Serialize Write ======================================

int DataTable::serializeWrite(void *d) const
{
	unsigned char*ptr = (unsigned char*)d;
	int pos = 0;

	/*version*/
	unsigned char s = 1;
	ptr[pos++] = s;

	/*number of types*/
	s = 3;
	ptr[pos++] = s;

	/*integers*/ 
    s = static_cast<unsigned char>(integers.size());
	ptr[pos++] = s;

	for (INTMAP::const_iterator i = integers.begin(); i!= integers.end(); i++)
	{
		/*id*/
		s = static_cast<unsigned char>(i->first.size());
		ptr[pos++] = s;

		memcpy(&ptr[pos], i->first.c_str() ,s);		
		pos += s;

		memcpy(&ptr[pos], &i->second, sizeof(int));
		pos += sizeof(int);
	}

	/*floats*/
	s = static_cast<unsigned char>(floats.size());
	ptr[pos++] = s;

	for (FLOATMAP::const_iterator f = floats.begin(); f!= floats.end(); f++)
	{
		/*id*/
		s = static_cast<unsigned char>(f->first.size());
		ptr[pos++] = s;

		memcpy(&ptr[pos], f->first.c_str() ,s);		
		pos += s;

		memcpy(&ptr[pos], &f->second, sizeof(float));
		pos += sizeof(float);
	}
	/*bools*/
	s = static_cast<unsigned char>(bools.size());
	ptr[pos++] = s;

	for (BOOLMAP::const_iterator b = bools.begin(); b!= bools.end(); b++)
	{
		/*id*/
		s = static_cast<unsigned char>(b->first.size());
		ptr[pos++] = s;

		memcpy(&ptr[pos], b->first.c_str() ,s);		
		pos += s;

		memcpy(&ptr[pos], &b->second, sizeof(bool));
		pos += sizeof(bool);
	}
	return pos; 
}
//== Serialize Read ======================================
int DataTable::serializeRead(void *d) 
{	
	unsigned char*ptr = (unsigned char*)d;
	int pos = 0;

	/*version*/ 
	unsigned char s = 1;
	s = ptr[pos++];

	int i,v;

	/*integers*/ 
	int count = ptr[pos++];

	for (i=0; i<count; i++)
	{
		/*id*/ 
		s = ptr[pos++];
		memcpy(_iobuffer, &ptr[pos], s);		
		_iobuffer[s] = '\0';
		pos += s;
		memcpy(&v, &ptr[pos], sizeof(int));
		pos += sizeof(int);

		/*update datatable*/ 
		set(_iobuffer, v);
	}	
	
	/*floats*/ 
	count = ptr[pos++];
	float fv;

	for (i=0; i<count; i++)
	{
		/*id*/ 
		s = ptr[pos++];
		memcpy(_iobuffer, &ptr[pos], s);	
		_iobuffer[s] = '\0';
		pos += s;
		memcpy(&fv, &ptr[pos], sizeof(float));
		pos += sizeof(float);

		/*update datatable*/ 
		set(_iobuffer, fv);
	}	

	/*bools*/ 
	count = ptr[pos++];
	bool bv;

	for (i=0; i<count; i++)
	{
		/*id*/ 
		s = ptr[pos++];
		memcpy(_iobuffer, &ptr[pos], s);	
		_iobuffer[s] = '\0';
		pos += s;
		memcpy(&bv, &ptr[pos], sizeof(bool));
		pos += sizeof(bool);

		/*update datatable*/ 
		set(_iobuffer, bv);
	}	
	return pos;
}
//== Serialize Size ======================================
int DataTable::serializeSize() const
{
	int sz = 5; /*constant*/ 

	/*integers*/ 
	for (INTMAP::const_iterator i = integers.begin(); i!= integers.end(); i++)
	{
		/*id*/ 
        sz += static_cast<int>(i->first.size() + 1);
		sz += sizeof(int);
	}
	/*floats*/ 
	for (FLOATMAP::const_iterator f = floats.begin(); f!= floats.end(); f++)
	{
		sz += static_cast<int>(f->first.size()+1);
		sz += sizeof(float);
	}
	/*bools*/ 
	for (BOOLMAP::const_iterator b = bools.begin(); b!= bools.end(); b++)
	{
		sz += static_cast<int>(b->first.size()+1);
		sz += sizeof(bool);
	}
	return sz;
}
