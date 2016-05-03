/*--------------------------------------------------------------------------*/ 
/*	CommandPipe interaction classes  definition								*/ 
/*  (C) 2003 Copyright Faraz Ravi - All Rights Reserved						*/ 
/*																			*/ 
/*  Last Updated 15 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef _COMMANDPIPE_INTERACT_DEFINITIONS
#define _COMMANDPIPE_INTERACT_DEFINITIONS

#pragma warning (disable : 4251)
#pragma warning (disable : 4275)

#include <stack>
#include <string>

#ifdef CMDPIPE_EXPORTS
#define CMDPIPE_API __declspec(dllexport)
#else
#define CMDPIPE_API __declspec(dllimport)
#endif

#include <pt/typedefs.h>
#include <pt/datatable.h>

namespace pt
{
	enum ResultType
	{
		RT_FLOAT,
		RT_BOOL,
		RT_INT,
		RT_PTR,
		RT_DOUBLE,
		RT_VECTOR3,
		RT_SSTRING,
		RT_DTPTR
	};
	/*--------------------------*/ 
	/*Result Structure			*/ 
	/*--------------------------*/ 
	class CmdResult
	{
	public:
		inline void setVal(const int &v) 		{ memcpy(_data, &v, sizeof(int)); type = RT_INT; }
		inline void setVal(const float &v) 		{ memcpy(_data, &v, sizeof(float)); type = RT_FLOAT; }
		inline void setVal(const double &v) 	{ memcpy(_data, &v, sizeof(double)); type = RT_DOUBLE; }
		inline void setPtr(void* v) 			{ memcpy(_data, &v, sizeof(void*)); type = RT_PTR; }
		inline void setVal(const bool &v) 		{ memcpy(_data, &v, sizeof(bool)); type = RT_BOOL; }
		inline void setVal(const float *v)		{ memcpy(_data, v, sizeof(float)*3); type = RT_VECTOR3; }
		inline void setVal(const char *v)		{ memcpy(_data, &v, sizeof(void*)); type = RT_SSTRING; }
		inline void setVal(pt::DataTable *v)	{ memcpy(_data, &v, sizeof(void*)); type = RT_DTPTR;; }

		CmdResult() { memset(_data, 0, sizeof(float)*3); }
		inline CmdResult(const int &v) 		{ setVal(v); }
		inline CmdResult(const bool &v) 	{ setVal(v); }
		inline CmdResult(const float &v) 	{ setVal(v); }
		inline CmdResult(const double &v) 	{ setVal(v); }
		inline CmdResult(const float* v)	{ setVal(v); }
		inline CmdResult(const char *v)		{ setVal(v); }
		inline CmdResult(pt::DataTable *&v)	{ setVal(v); }
		
		inline bool getVal(int &v)  		const	{ if (type != RT_INT) return false;		memcpy(&v, _data, sizeof(int)); return true; }
		inline bool getVal(float &v)		const 	{ if (type != RT_FLOAT) return false;	memcpy(&v, _data, sizeof(float)); return true; }
		inline bool getVal(double &v) 		const	{ if (type != RT_DOUBLE) return false;	memcpy(&v, _data, sizeof(double)); return true; }
		inline bool getVal(bool &v) 		const	{ if (type != RT_BOOL) return false;	memcpy(&v, _data, sizeof(bool)); return true; }
		inline bool getVal(pt::DataTable *&v) const	{ if (type != RT_DTPTR) return false;	memcpy(&v, _data, sizeof(void*)); return true; }
		inline bool getPtr(void *&v) 		const	{ if (type != RT_PTR) return false;		memcpy(&v, _data, sizeof(void*)); return true; }
		inline bool getVal(float *v) 		const	{ if (type != RT_VECTOR3) return false; memcpy(v, _data, sizeof(float)*3); return true; }
		inline bool getVal(const char *&v) 	const	{ if (type != RT_SSTRING) return false; memcpy(&v, _data, sizeof(void*)); return true; }
		
		CmdResult &operator = (const CmdResult &r)
		{
			memcpy(_data, r._data, sizeof(float)*3);
			type = r.type;
			return *this;
		}
			ResultType type;
		private:
			unsigned char _data[sizeof(float)*3];
	};
	/*--------------------------*/ 
	/*Args Stack				*/ 
	/*--------------------------*/ 
	class CmdArgs
	{
	public:
		void pushArg(const int &v)		{ _args.push(CmdResult(v)); }
		void pushArg(const float &v)	{ _args.push(CmdResult(v)); }
		void pushArg(const float *v)	{ _args.push(CmdResult(v)); }
		void pushArg(const double &v)	{ _args.push(CmdResult(v)); }
		void pushArg(const bool &v)		{ _args.push(CmdResult(v)); }
		void pushArg(const char *v)		{ _args.push(CmdResult(v)); }
		void pushArg(pt::DataTable *v)		{ _args.push(CmdResult(v)); }
		void pushPtrArg(void *v)	{ CmdResult cr; cr.setPtr(v); _args.push(cr); }

		uint size() const { return _args.size(); }

		bool popArg(int &v)				{	CmdResult r; if (popArg(r, RT_INT))		{ return r.getVal(v); } return false; }
		bool popArg(bool &v)			{	CmdResult r; if (popArg(r, RT_BOOL))	{ return r.getVal(v); } return false; }
		bool popArg(float &v)			{	CmdResult r; if (popArg(r, RT_FLOAT))	{ return r.getVal(v); } return false; }
		bool popArg(double &v)			{	CmdResult r; if (popArg(r, RT_DOUBLE))	{ return r.getVal(v); } return false; }
		bool popArg(const char *&v)		{	CmdResult r; if (popArg(r, RT_SSTRING)) { return r.getVal(v); } return false; }
		bool popArg(pt::DataTable *&v)	{	CmdResult r; if (popArg(r, RT_DTPTR))	{ return r.getVal(v); } return false; }
		bool popArg(float *v)			{	CmdResult r; if (popArg(r, RT_VECTOR3)) { return r.getVal(v); } return false; }
		bool popPtrArg(void *v)			{	CmdResult r; if (popArg(r, RT_PTR))		{ return r.getPtr(v); } return false; }

		CMDPIPE_API bool popArg(CmdResult &res, ResultType t);
		void clear() { while (_args.size()) _args.pop(); }
	private:
		std::stack<CmdResult> _args;
	};

}
#endif