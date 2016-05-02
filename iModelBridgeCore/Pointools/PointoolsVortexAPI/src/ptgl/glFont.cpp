//*******************************************************************
//glfont2.cpp -- glFont Version 2.0 implementation
//Copyright (c) 1998-2002 Brad Fish
//See glfont.html for terms of use
//May 14, 2002
//*******************************************************************
#include "PointoolsVortexAPIInternal.h"

#include <GL/gl.h>

#include <string>
#include <utility>
#include <iostream>
#include <fstream>

using namespace std;

#include <ptgl/glFont.h>

using namespace ptgl;

//*******************************************************************
//Font Class Implementation
//*******************************************************************
Font::Font ()
{
	/*Initialize header to safe state*/ 
	header.tex = -1;
	header.tex_width = 0;
	header.tex_height = 0; 
	header.start_char = 0;
	header.end_char = 0;
	header.chars = NULL;
}
//*******************************************************************
Font::~Font ()
{
	//Destroy the font
	destroy();
}
//*******************************************************************
bool Font::create (const char *file_name, int tex, GLuint mag_filter, GLuint min_filter)
{
	ifstream input;
	int num_chars, num_tex_bytes;
	char *tex_bytes;

	//Destroy the old font if there was one, just to be safe
	destroy();

	//Open input file
	input.open(file_name, ios::in | ios::binary);
	if (!input)
		return false;

	//Read the header from file
	input.read((char *)&header, sizeof(header));
	header.tex = tex;

	//Allocate space for character array
	num_chars = header.end_char - header.start_char + 1;
	if ((header.chars = new GLFontChar[num_chars]) == NULL)
		return false;

	//Read character array
	input.read((char *)header.chars, sizeof(GLFontChar) *
		num_chars);

	//Read texture pixel data
	num_tex_bytes = header.tex_width * header.tex_height * 2;
	tex_bytes = new char[num_tex_bytes];
	input.read(tex_bytes, num_tex_bytes);

	//Create OpenGL texture
	glBindTexture(GL_TEXTURE_2D, tex);  
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 2, header.tex_width,
		header.tex_height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE,
		(void *)tex_bytes);

	//Free texture pixels memory
	delete[] tex_bytes;

	//Close input file
	input.close();

	//Return successfully
	return true;
}
//*******************************************************************
bool Font::create (const std::string &file_name, int tex, GLuint mag_filter, GLuint min_filter)
{
	return create(file_name.c_str(), tex, mag_filter, min_filter);
}
//*******************************************************************
void Font::destroy (void)
{
	//Delete the character array if necessary
	if (header.chars)
	{
		delete[] header.chars;
		header.chars = NULL;
	}
}
//*******************************************************************
void Font::getTexSize (std::pair<int, int> *size)
{
	//Retrieve texture size
	size->first = header.tex_width;
	size->second = header.tex_height;
}
//*******************************************************************
int Font::getTexWidth (void)
{
	//Return texture width
	return header.tex_width;
}
//*******************************************************************
int Font::getTexHeight (void)
{
	//Return texture height
	return header.tex_height;
}
//*******************************************************************
void Font::getCharInterval (std::pair<int, int> *interval)
{
	//Retrieve character interval
	interval->first = header.start_char;
	interval->second = header.end_char;
}
//*******************************************************************
int Font::getStartChar (void)
{
	//Return start character
	return header.start_char;
}
//*******************************************************************
int Font::getEndChar (void)
{
	//Return end character
	return header.end_char;
}
//*******************************************************************
void Font::getCharSize (int c, std::pair<int, int> *size)
{
	//Make sure character is in range
	if (c < header.start_char || c > header.end_char)
	{
		//Not a valid character, so it obviously has no size
		size->first = 0;
		size->second = 0;
	}
	else
	{
		GLFontChar *glfont_char;

		//Retrieve character size
		glfont_char = &header.chars[c - header.start_char];
		size->first = (int)(glfont_char->dx * header.tex_width);
		size->second = (int)(glfont_char->dy *
			header.tex_height);
	}
}
//*******************************************************************
int Font::getCharWidth (int c)
{
	//Make sure in range
	if (c < header.start_char || c > header.end_char)
		return 0;
	else
	{
		GLFontChar *glfont_char;
		
		//Retrieve character width
		glfont_char = &header.chars[c - header.start_char];
		return (int)(glfont_char->dx * header.tex_width);
	}
}
//*******************************************************************
int Font::getCharHeight (int c)
{
	//Make sure in range
	if (c < header.start_char || c > header.end_char)
		return 0;
	else
	{
		GLFontChar *glfont_char;

		//Retrieve character height
		glfont_char = &header.chars[c - header.start_char];
		return (int)(glfont_char->dy * header.tex_height);
	}
}
//*******************************************************************
void Font::begin (void)
{
	//Bind to font texture
	glBindTexture(GL_TEXTURE_2D, header.tex);
}
//*******************************************************************

//End of file

