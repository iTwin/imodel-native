#pragma once
//
// 3smGL  Interface
//
// Helper for OpenGL display of ScalableMesh 3sm files
// (c) Copyright Bentley Systems 2017
// All Rights Reserved
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#ifdef _3SMGL_API_EXPORT
	#define  _3SMGL_API  __declspec(dllexport)
#else
	#define _3SMGL_API	__declspec(dllimport)
#endif

#include <string>

// Implementation hiding, minimize deps
typedef void * SMPointer;
typedef void * SMRenderer;

// Basic camera definition
struct _3SMGL_API GL_Camera
{
	int		viewport[4];	// x,y,w,h
	double	position[3];	// eye x,y,z
	double  target[3];		// target x,y,z
	float	fov;			// in degrees
	double	far_plane;		
	double	near_plane;
};

class  _3SMGL_API GL_ScalableMesh
{
public:
	GL_ScalableMesh(const char *filepath);
	~GL_ScalableMesh();

	bool isValid() const;						// 3sm correctly loaded
	
	bool draw(GL_Camera &camera, bool faces=true, bool texture=true, bool wire = false, bool boxes=false);				// returns true if draw was complete, false if partial
	
	void getBounds(double &x, double &y, double &z, double &dx, double &dy, double &dz);
	void getCenter(double &x, double &y, double &z);

	void setUniformTexID( unsigned int id );	// texture unit 

	float getAverageDisplayLod() const;				// stats

	// simple coordinate locale management 
	static void setWorldOffset(double x, double y, double z);	
	static void getWorldOffset(double &x, double &y, double &z);

private:
	SMPointer load3sm(const std::string &filepath);
	bool unload3sm(SMPointer sm);

	double		m_dims[6];
	int			m_texUnit;
	SMPointer	m_sm;
	SMRenderer  m_renderer;
	char		m_filename[MAX_PATH];
};
