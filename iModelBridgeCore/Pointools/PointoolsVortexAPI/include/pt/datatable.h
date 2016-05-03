/*--------------------------------------------------------------------------*/ 
/*  Datatable.h																*/ 
/*	Datatable base class definition	- run time struct definition			*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 1 Jan 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef COMMONCLASSES_DATATABLE_DEFINITION
#define COMMONCLASSES_DATATABLE_DEFINITION

#include <pt/classes.h>
#include <pt/geomtypes.h>
#include <pt/ptstring.h>

#include <Loki/AssocVector.h>

#include <string>

#pragma warning (disable : 4800)

namespace pt
{
	typedef std::string DTID;
	#define DT_MAPTYPE Loki::AssocVector

	enum TYPE
	{
		NOTYPE,
		BOOLTYPE,
		FLOATTYPE,
		DOUBLETYPE,
		INTTYPE,
		VECTOR3TYPE,
		STRINGTYPE
	};

class CCLASSES_API DataTable
{
public:
	DataTable();
	~DataTable();

	/*define*/ 
	void addi(const DTID &id, int val = 0);
	void addf(const DTID &id, float val = 0);		
	void addb(const DTID &id, bool val = false);		
	void addv(const DTID &id, const vector3 &val = vector3(0,0,0)); 
	void adds(const DTID &id, const char* val=0);

	void addP(const DTID &id, int *ptr);
	void addP(const DTID &id, float *ptr);
	void addP(const DTID &id, bool *ptr);
	void addP(const DTID &id, vector3 *ptr);
	void addP(const DTID &id, String *ptr);

	void clear();
	int size();

	TYPE type(const DTID &id);

	/*mutation*/ 
	bool set(const DTID &id, const bool &val);
	bool set(const DTID &id, const float &val);
	bool set(const DTID &id, const int &val);
	bool set(const DTID &id, const char *str);
	bool set(const DTID &id, const vector3 &val);

	/*exists*/ 
	bool hasb(const DTID &id);
	bool hasf(const DTID &id);
	bool hasi(const DTID &id);
	bool hass(const DTID &id);
	bool hasv(const DTID &id);

	/*access*/ 
	bool get(const DTID &id, bool &val);
	bool get(const DTID &id, float &val);
	bool get(const DTID &id, int &val);
	bool get(const DTID &id, String &val);
	bool get(const DTID &id, vector3 &val);

	DataTable &operator = (const DataTable &dt);

	int serializeRead(void *);
	int serializeWrite(void *) const;
	int serializeSize() const;

private:

	/*different map for every type - simplest + quickest typesafe solution*/ 
	typedef DT_MAPTYPE<DTID, int>			INTMAP; 	INTMAP		integers;
	typedef DT_MAPTYPE<DTID, float>			FLOATMAP; 	FLOATMAP	floats;
	typedef DT_MAPTYPE<DTID, bool>			BOOLMAP; 	BOOLMAP		bools;
	typedef DT_MAPTYPE<DTID, String>		STRINGMAP; 	STRINGMAP	strings;
	typedef DT_MAPTYPE<DTID, vector3>		VEC3MAP; 	VEC3MAP		vectors;

	typedef DT_MAPTYPE<DTID, int*>			PINTMAP; 	PINTMAP		pintegers;
	typedef DT_MAPTYPE<DTID, float*>		PFLOATMAP; 	PFLOATMAP	pfloats;
	typedef DT_MAPTYPE<DTID, bool*>			PBOOLMAP; 	PBOOLMAP	pbools;
	typedef DT_MAPTYPE<DTID, String*>		PSTRINGMAP; PSTRINGMAP	pstrings;
	typedef DT_MAPTYPE<DTID, vector3*>		PVEC3MAP; 	PVEC3MAP	pvectors;
};
}
#endif
