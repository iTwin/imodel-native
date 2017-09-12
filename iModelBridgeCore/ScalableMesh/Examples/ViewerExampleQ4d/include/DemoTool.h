#pragma once

/******************************************************************************

Pointools Vortex API Examples

NavigationTool.h

Provides basic camera navigation for example applications. Does not demonstrate
any Vortex features

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/

#include "ExampleApp.h"

#include <vector>

struct UIButton
{
	UIButton();
	
	bool		loadResource();
	void		draw(double scale=1.0);
	void		test_setup();

	std::string resource_file;
	bool		loaded;
	int			x_pos;
	int			y_pos;
	int			width;
	int			height;
	GLuint		texID;
	
	Vector3f	zoom_to_cam;
	Vector3f	zoom_to_tar;
};

class SM_DemoTool : public Tool
{
public:
	SM_DemoTool();

	void parseCommandLine(int argc, char* argv[]);

	bool onMouseButtonDown(int button, int x, int y);
	bool onMouseButtonUp(int button, int x, int y);
	bool onMouseDrag(int x, int y, int startX, int startY);
	
	void preDrawSetup();
	void drawPreDisplay();
	void drawPostDisplay();

	static SM_DemoTool *instance();

private:
	struct LoadObject
	{
		LoadObject(std::string file, float point_size = -1) : src(file), pointSize(point_size) {}
		std::string	src;
		float		pointSize;	// unused
	};
	double							m_uiscale;
	std::vector<UIButton*>			m_buttons;
	std::vector<LoadObject>			m_objects;

	std::string	m_configFile;
	static		SM_DemoTool*	s_instance;

	void		readUIConfig();
	void		loadObjects();

	bool		m_doCameraMove;
	Vector3f	m_camFrom;
	Vector3f	m_camTo;
	Vector3f	m_tarFrom;
	Vector3f	m_tarTo;
	
	float		m_pos;

};



