#include <gl/glew.h>
#include "NavigationTool.h"
#include "DemoTool.h"
#include "3SMGL.h"
#include "png.h"
#include "config.h"

// cmdline parser
#include "cmdparser.h"

//-----------------------------------------------------------------------------
UIButton::UIButton()
//-----------------------------------------------------------------------------
{
	loaded = false;
	x_pos = 0;
	y_pos = 0;
	width = 0;
	height = 0;
	texID = 0;
	zoom_to_cam.zero();
	zoom_to_tar.zero();
}
//-----------------------------------------------------------------------------
void UIButton::test_setup()
//-----------------------------------------------------------------------------
{
	x_pos = 50;
	y_pos = 100;

	zoom_to_cam.set(-10.4145f, -15.8796f, -8.9966f);
	zoom_to_tar.set(3.81352f, 12.7203f, -8.80998f);

	resource_file = "data\\start.png";
}
//-----------------------------------------------------------------------------
std::wstring str2wstr(const std::string& s) 
//-----------------------------------------------------------------------------
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
//-----------------------------------------------------------------------------
bool UIButton::loadResource()
//-----------------------------------------------------------------------------
{
	if (loaded) return true;

	if (texID)
	{
		glDeleteTextures(1, &texID);
	}
	texID = GLHelper::loadTexturePNG(resource_file, width, height);

	if (texID)
	{
		loaded = true;
	}
	return true;
}
//-----------------------------------------------------------------------------
void UIButton::draw(double scale)
//-----------------------------------------------------------------------------
{
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
	
	int xp = x_pos * scale;
	int yp = y_pos * scale;

	if (x_pos < 0) xp = tw + xp;
	if (y_pos < 0) yp = th + yp;

	GLenum err = glGetError();

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glEnable(GL_COLOR_MATERIAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	 err = glGetError();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glColor3f(1, 1, 1);
	glTranslatef(0, 0, -1);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluOrtho2D(tx, tx + tw, ty, ty + th);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, texID);
	err = glGetError();
	//glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);	

	glBegin(GL_QUADS);

		glTexCoord3i(0, 0, 0);
		glVertex2i(xp, yp);

		glTexCoord3i(1, 0, 0);
		glVertex2i(xp+width* scale, yp);

		glTexCoord3i(1, 1, 0);
		glVertex2i(xp+width* scale, yp+height* scale);

		glTexCoord3i(0, 1, 0);
		glVertex2i(xp, yp + height* scale);

	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);	
	
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}
//-----------------------------------------------------------------------------
SM_DemoTool* SM_DemoTool::s_instance = 0;
//-----------------------------------------------------------------------------
SM_DemoTool::SM_DemoTool() 
//-----------------------------------------------------------------------------
{
	s_instance = this;
	m_doCameraMove = false;
	m_uiscale = 1.0;
	m_configFile = "viewer_config.txt";
}
//-----------------------------------------------------------------------------
void SM_DemoTool::parseCommandLine(int argc, char * argv[])
//-----------------------------------------------------------------------------
{
	std::stringstream output{};
	std::stringstream errors{};

	cli::Parser parser(argc, argv);

	parser.set_optional<std::string>("o", "open", "test.3sm", "ScalableMesh 3sm file to open");
	parser.set_optional<std::string>("c", "config", "viewer_config.txt", "Configuration file to load");
	parser.set_optional<bool>("fs", "fullscreen", false, "Run the viewer full screen");

	if (parser.run(output, errors) == false)
	{
		return;
	}

	std::string file_open = parser.get<std::string>("o");
	m_configFile = parser.get<std::string>("c");
	m_objects.push_back(LoadObject(file_open, 0));

	bool fs = parser.get<bool>("fs");

	if (fs) glutFullScreen();
}
//-----------------------------------------------------------------------------
bool SM_DemoTool::onMouseButtonDown(int button, int x, int y)
//-----------------------------------------------------------------------------
{
	int tx, ty, tw, th;
	getViewportSize(tx, ty, tw, th);

	// check buttons
	for (int i = 0; i < m_buttons.size(); i++)
	{	
		int xp = x - m_buttons[i]->x_pos * m_uiscale;
		int yp = (th-y) - m_buttons[i]->y_pos * m_uiscale;

		if (xp > 0 && xp < m_buttons[i]->width * m_uiscale && yp > 0 && yp < m_buttons[i]->height * m_uiscale)
		{
			if (!m_buttons[i]->zoom_to_cam.is_zero())
			{
				m_camFrom = CameraNavigation::instance()->getCameraPosition();
				m_tarFrom = CameraNavigation::instance()->getCameraTarget();
				m_camTo = m_buttons[i]->zoom_to_cam;
				m_tarTo = m_buttons[i]->zoom_to_tar;
				m_pos = 0;

				startDynamicView();

				m_doCameraMove = true;
				viewRedraw();
				return true;
			}
		}
	}
	return false;
}
//-----------------------------------------------------------------------------
bool SM_DemoTool::onMouseButtonUp(int button, int x, int y)
//-----------------------------------------------------------------------------
{
	return false;
}
//-----------------------------------------------------------------------------
bool SM_DemoTool::onMouseDrag(int x, int y, int startX, int startY) 
//-----------------------------------------------------------------------------
{
	return false;
}
//-----------------------------------------------------------------------------
void SM_DemoTool::readUIConfig()
//-----------------------------------------------------------------------------
{
	using namespace std;

	string configpath = ExampleApp::instance()->getFullResourcePath( m_configFile );

	Config config(configpath);

	// get properties for all subgroups starting with prefix
	map<string, Config*> buttons = config.getGroups(); // all groups

	const string button_prefix = "Button"; // prefix for group name
	const string object_prefix = "Object"; // prefix for group name

	m_uiscale = config.hasSymbol("ui_scale") ? config.pDouble("ui_scale") : 1.0;

	if (config.pString("window_format")=="Fullscreen" )
		glutFullScreen();
	 
	const GLfloat *horizon = ExampleApp::instance()->getView().backColor;

	std::string back_col = config.pString("back_col");
	sscanf(back_col.c_str(), "%f, %f, %f", &horizon[0], &horizon[1], &horizon[2]);

	if (config.hasSymbol("camera_near_plane"))
	{
		double plane = config.pDouble("camera_near_plane");
		if (plane > 0 && plane < 1000)
		{
			CameraNavigation::instance()->setCameraNearPlane(plane);
		}
	}
	if (config.hasSymbol("camera_far_plane"))
	{
		double plane = config.pDouble("camera_far_plane");
		if (plane > 0 && plane < 10000)
		{
			CameraNavigation::instance()->setCameraFarPlane(plane);
		}
	}
	if (config.hasSymbol("camera_fov"))
	{
		double fov = config.pDouble("camera_fov");
		if (fov > 10 && fov < 300)
		{
			CameraNavigation::instance()->setCameraFov(fov);
		}
	}
	std::string cam_pos = config.pString("camera_position");

	if (cam_pos.length())
	{
		Vector3f pos;
		sscanf(cam_pos.c_str(), "%f, %f, %f", &pos.x, &pos.y, &pos.z);
		CameraNavigation::instance()->setCameraPosition(pos);
	}
	std::string cam_tar = config.pString("camera_target");

	if (cam_tar.length())
	{
		Vector3f tar;
		sscanf(cam_tar.c_str(), "%f, %f, %f", &tar.x, &tar.y, &tar.z);
		CameraNavigation::instance()->setCameraTarget(tar);
	}

	for (map<string, Config*>::iterator i = buttons.begin(); i != buttons.end(); ++i) 
	{
		string groupName = i->first;
		Config* group = i->second;

		// test group name for prefix
		if (groupName.substr(0, button_prefix.length()) == button_prefix)
		{
			UIButton *b = new UIButton;
			b->resource_file = group->pString("img");
			b->x_pos = group->pInt("x");
			b->y_pos = group->pInt("y");

			std::string cam_pos = group->pString("camera_position");

			if (cam_pos.length())
			{
				sscanf(cam_pos.c_str(), "%f, %f, %f", &b->zoom_to_cam.x, &b->zoom_to_cam.y, &b->zoom_to_cam.z);
			}
			std::string cam_tar = group->pString("camera_target");

			if (cam_tar.length())
			{
				sscanf(cam_tar.c_str(), "%f, %f, %f", &b->zoom_to_tar.x, &b->zoom_to_tar.y, &b->zoom_to_tar.z);
			}

			m_buttons.push_back(b);
		}
		else if (groupName.substr(0, object_prefix.length()) == object_prefix)
		{
			string groupName = i->first;
			Config* group = i->second;

			m_objects.push_back( LoadObject(group->pString("src"), 0) );
		}
	} 
}
//-----------------------------------------------------------------------------
void SM_DemoTool::loadObjects()
//-----------------------------------------------------------------------------
{
	int number_3sm = 0;

	Vector3d cen;
	cen.zero();

	Vector3d cen_av;
	cen_av.zero();

	for (int i = 0; i < m_objects.size(); i++)
	{
		if (m_objects[i].src.find(".3sm") != std::string::npos)
		{ 
			GL_ScalableMesh * mesh = ExampleApp::instance()->addScalableMesh(m_objects[i].src);
			if (mesh->isValid())
			{
				number_3sm++;
				mesh->getCenter(cen.x, cen.y, cen.z);
				cen_av += cen;
			}
		}
	}	
	if (number_3sm)
	{
		cen_av /= number_3sm;
	}
	GL_ScalableMesh::setWorldOffset(cen_av.x, cen_av.y, cen_av.z);
}
//-----------------------------------------------------------------------------
void SM_DemoTool::preDrawSetup()
//-----------------------------------------------------------------------------
{
	if (m_buttons.size() == 0)
	{
		readUIConfig();	// needs a valid GL context
		loadObjects();
	}
}
//-----------------------------------------------------------------------------
void SM_DemoTool::drawPreDisplay()
//-----------------------------------------------------------------------------
{

}
//-----------------------------------------------------------------------------
void SM_DemoTool::drawPostDisplay()
//-----------------------------------------------------------------------------
{	
	for (int i = 0; i < m_buttons.size(); i++)
	{
		//m_buttons[i]->loaded = false;
		m_buttons[i]->loadResource();

		m_buttons[i]->draw(m_uiscale);
	}

	Vector3f cam_pos = CameraNavigation::instance()->getCameraPosition();
	Vector3f cam_tar = CameraNavigation::instance()->getCameraTarget();

//	std::cout << "pos = " << cam_pos.x << ", " << cam_pos.y << ", " << cam_pos.z << std::endl;
//	std::cout << "tar = " << cam_tar.x << ", " << cam_tar.y << ", " << cam_tar.z << std::endl;

	if (m_doCameraMove)
	{
		m_pos += 0.05f;
		if (m_pos > 1.0f)
		{
			m_doCameraMove = false;
			m_pos = 0;
			endDynamicView();
			CameraNavigation::instance()->setCameraPosition(m_camTo);
			CameraNavigation::instance()->setCameraTarget(m_tarTo);
			viewRedraw();
		}
		else
		{
			Vector3f cam_pos = m_camFrom + (m_camTo - m_camFrom) * (1-cos(m_pos*3.142))*0.5;
			Vector3f tar_pos = m_tarFrom + (m_tarTo - m_tarFrom) * (1-cos(m_pos*3.142))*0.5;

			CameraNavigation::instance()->setCameraPosition(cam_pos);
			CameraNavigation::instance()->setCameraTarget(tar_pos);

			viewRedraw();
		}
	}
}
//-----------------------------------------------------------------------------
SM_DemoTool *SM_DemoTool::instance()
//-----------------------------------------------------------------------------
{
	return s_instance;
}
