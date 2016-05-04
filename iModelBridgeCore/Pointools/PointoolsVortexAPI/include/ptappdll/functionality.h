/*--------------------------------------------------------------------------*/ 
/*	Pointools App class class												*/ 
/*  (C) 2005 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*--------------------------------------------------------------------------*/ 

#define PT_STEREO_FUNCTIONALITY 0xa48fe30681b49c06

namespace pt {class ParameterMap; }

namespace ptapp
{
	typedef uint64_t FuncCode;

	void addFunctionality(void*);
	pt::ParameterMap *functionality(FuncCode);
}