/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/ 
/*	Pointools App class class												*/ 
/*--------------------------------------------------------------------------*/ 

#define PT_STEREO_FUNCTIONALITY 0xa48fe30681b49c06

namespace pt {class ParameterMap; }

namespace ptapp
{
	typedef uint64_t FuncCode;

	void addFunctionality(void*);
	pt::ParameterMap *functionality(FuncCode);
}
