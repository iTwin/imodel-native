/*--------------------------------------------------------------------------*/ 
/*	Pointools GLtext class definition and implementation					*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 15 April 2004Faraz Ravi									*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef POINTOOLS_GLTEXT_INTERFACE
#define POINTOOLS_GLTEXT_INTERFACE

#include <ptgl/ptgl.h>
#include <pt/rect.h>
#include <stdarg.h>
#include <cstdio>

namespace ptgl
{
	class PTGL_API Text
	{
	public: 
		static bool setFont(const char *font);
		static void	getSize(const char *text, pt::Recti &r);
		static void beginText();
		static void textOutf(int xpos, int ypos, const char *format, ...);
		static void textOut(int xpos, int ypos, const char *txt);
		static void destroy();
	};
}
#endif

