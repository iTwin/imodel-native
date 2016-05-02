#pragma warning ( disable : 4786)
#include "PointoolsVortexAPIInternal.h"

#include <gl/gl.h>
#include <gl/glu.h>

#ifdef POINTOOLS_API_INCLUDE
#include <ptapi/PointoolsVortexAPI.h>
#endif

#include <ptgl/gltext.h>
#include <ptgl/glFont.h>
#include <ptappdll/ptapp.h>
#include <pt/UnicodeConversion.h>

#include <iostream>
#include <map>
#include <string>

using namespace ptgl;
typedef std::map<std::string, Font*> FONTMAP;

namespace _ptglfntman
{
	class FontManager 
	{
	public:
		FontManager() : _font(0) {};
		~FontManager();

		FONTMAP fonts;
		Font *_font;
	};
	typedef std::map<HDC, FontManager*> FONTMANAGERS;
	FONTMANAGERS _fmrc;
	
	FontManager *fontManager(HDC dc)
	{
		if (!dc) return 0;
		FONTMANAGERS::iterator it = _fmrc.find(dc);
		if (it != _fmrc.end()) return it->second;
		else
		{
			FontManager *f = new FontManager(); 
			_fmrc.insert(FONTMANAGERS::value_type(dc, f)).second;
			return f;
		}
	}
}
using namespace _ptglfntman;
//
// set font
//
bool Text::setFont(const char *font)
{	
	FontManager *fm = fontManager(wglGetCurrentDC());
	if (!fm) return false;

	FONTMAP::iterator it= fm->fonts.find(font);

	/*check existing*/ 
	if (it != fm->fonts.end())
	{
		fm->_font = it->second;
		return true;
	}
	/*try create new*/ 
	wchar_t filename[260];
	GLuint tid;
	glGenTextures(1, &tid);
	
#ifdef POINTOOLS_API_INCLUDE 
	#ifndef POINTOOLS_API
		const wchar_t *app = ptGetWorkingFolder();
	#else
		const wchar_t *app = ptapp::apppath();
	#endif
#else
	const wchar_t *app = ptapp::apppath();
#endif
	swprintf(filename, L"%s\\fonts\\%s.glf", app, pt::Ascii2Unicode::convert(font).c_str());

	Font* _oldfont = fm->_font;
	fm->_font = new Font;

	if (fm->_font->create(pt::Unicode2Ascii::convert(filename).c_str(), 
		tid, GL_NEAREST, GL_NEAREST))
	{
		fm->fonts.insert(FONTMAP::value_type(font, fm->_font));
		//std::cout << "Creating font " << font << std::endl;
		return true;
	}
	else
	{
		//std::cout << "Font create failed : Font file: " << filename << std::endl;
		delete fm->_font;
		fm->_font = _oldfont;
		return false;	
	}
}	
void Text::beginText()
{
	FontManager *fm = fontManager(wglGetCurrentDC());
	if (fm && fm->_font) fm->_font->begin();
}
void Text::textOutf(int xpos, int ypos, const char *format, ...)
{
	
}
void Text::textOut(int xpos, int ypos, const char *txt)
{
	FontManager *fm = fontManager(wglGetCurrentDC());
	if (fm && fm->_font) fm->_font->draw(txt, xpos, ypos);
}
void Text::getSize(const char *text, pt::Recti &_r)
{
	pt::Rectf r;

	FontManager *fm = fontManager(wglGetCurrentDC());
	if (!fm || !fm->_font) return;

	int size = strlen(text);
	r.lx() = 0;
	r.ly() = 0;
	r.ux() = 0;
	r.uy() = 0;
	int mh = 0;
	int mw = 0;


	for (int i=0; i<size; i++)
	{
		if (text[i] == '\n')
		{
			r.uy() -= fm->_font->getCharHeight('g')+1;
			if (r.ux() > mw) mw = r.ux();
			r.ux() = 0;
		}
		else
		{
			r.ux() +=fm->_font->getCharWidth(text[i])+0.75f;
			int h = fm->_font->getCharHeight(text[i]);
			if (h>mh) mh = h;
		}
	}
	if (r.ux() > mw) mw = r.ux();

	r.uy() -= mh + 5;
	r.ly() += 5;
	r.ux() = mw + 5;
	r.lx() = 0;
	r.makeValid();

	_r.lx() = r.lx();
	_r.ux() = r.ux();
	_r.ly() = r.ly();
	_r.uy() = r.uy();
}

