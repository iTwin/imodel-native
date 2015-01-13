/******************************************************************************

Pointools Vortex API Examples

ClippingTool.cpp

Demonstrates layer based point editing capabilities of Vortex

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/
#include <GL/glui.h>
#include "ClippingTool.h"
#include <math.h>
#include "../include/PointoolsVortexAPI_resultCodes.h"
#include <assert.h>

static RGBc layerButtonCol(96,96,96);
static RGBc layerButtonSelCol(35,174,183);
static RGBc layerButtonVisCol(255, 86, 91);
#define PI 3.14159265

//-----------------------------------------------------------------------------
ClippingTool::ClippingTool(bool simple) : Tool(CmdFitPlanesToBB, CmdToggleClipping)
//-----------------------------------------------------------------------------
{
	m_planeDefList = NULL;
	m_fitPlanesButton = NULL;
	m_clipStyleButton = NULL;
	m_clipStyle = PT_CLIP_OUTSIDE;
	m_togglePlaneButton = NULL;
	m_updatePlaneButton = NULL;
	m_toggleClipping = NULL;
	m_clippingOn = true;
	
}
//-----------------------------------------------------------------------------
void ClippingTool::command( int cmdId )
//-----------------------------------------------------------------------------
{
	switch (cmdId)
	{

		case CmdFitPlanesToBB: 
			fitPlanesToBB(); 
			viewRedraw();
			break;		

		case CmdToggleClipStyle:
			if (m_clipStyle == PT_CLIP_OUTSIDE)
				m_clipStyle = PT_CLIP_INSIDE;
			else
				m_clipStyle = PT_CLIP_OUTSIDE;
			ptSetClipStyle(m_clipStyle);
			viewRedraw();
			break;

		case CmdUpdatePlane:		
			if (m_planeDefList)
			{
				int selected = m_planeDefList->get_current_item();
				updatePlane(selected);
				viewRedraw();
			}
			break;		

		case CmdSelectPlaneDef:		
			if (m_planeDefList)
			{
				int selected = m_planeDefList->get_current_item();
				planeDefSelected(selected);
				viewRedraw();
			}
			break;		

		case CmdTogglePlane:
			if (m_planeDefList)
			{
				int selected = m_planeDefList->get_current_item();
				toggleSelectedPlane(selected);
				viewRedraw();
			}
			break;

		case CmdToggleClipping:
			enableClipping(!m_clippingOn);			
			updateToggleClippingButtonText();
			viewRedraw();
			break;
	}
}
//-----------------------------------------------------------------------------
bool	ClippingTool::onMouseButtonDown( int button, int x, int y )
//-----------------------------------------------------------------------------
{
	return false;
}
//-----------------------------------------------------------------------------
bool	ClippingTool::onMouseButtonUp( int button, int x, int y )
//-----------------------------------------------------------------------------
{
	bool handled = false;


	return handled;
}
//-----------------------------------------------------------------------------
bool	ClippingTool::onMouseMove( int x, int y )
//-----------------------------------------------------------------------------
{
	return false;
}


//-----------------------------------------------------------------------------
bool	ClippingTool::onMouseDrag( int x, int y, int startX, int startY )
//-----------------------------------------------------------------------------
{
	return false;
}
//-----------------------------------------------------------------------------
void	ClippingTool::drawPostDisplay()
//-----------------------------------------------------------------------------
{
	vortex::Vector3d pt0, pt1, pt2, pt3;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLfloat savedColour[4] = {0.0f};
	glGetFloatv(GL_CURRENT_COLOR, savedColour);

	int selected = -1;
	if (m_planeDefList)	
		selected = m_planeDefList->get_current_item();
	
	// Draw the planes
	if (m_clippingOn)
	{
		for (int i = 0; i < CLIPPING_TOOL_NUM_PLANES; i++)
		{			
			if (ptIsClippingPlaneEnabled(i))
			{				
				// calculate plane corners
				// TODO: the u,v values of -10 and 10 here are arbitrary and should be updated to
				// take into account the scene size
				m_planes[i].to3D(-10, -10, pt0);
				m_planes[i].to3D(-10, 10, pt1);
				m_planes[i].to3D(10, -10, pt2);
				m_planes[i].to3D(10, 10, pt3);		

				if (i == selected)
					glColor4f(1.0f, 0.5f, 0.0f, 0.5f);
				else
					glColor4f(0.0f, 0.5f, 1.0f, 0.5f);
							
				glBegin(GL_QUADS); 		
				glVertex3f(pt0.x, pt0.y, pt0.z);		
				glVertex3f(pt1.x, pt1.y, pt1.z);				
				glVertex3f(pt3.x, pt3.y, pt3.z);		
				glVertex3f(pt2.x, pt2.y, pt2.z);		
				glEnd(); 

				// draw the plane normal
				vortex::Vector3d ptn = (pt0+pt3)/2;
				glColor4f(1.0f, 0.5f, 0.0f, 1.0f);

				glBegin(GL_LINES); 		
				glVertex3f(ptn.x, ptn.y, ptn.z);		
				glVertex3f(ptn.x+m_planes[i].normal().x, ptn.y+m_planes[i].normal().y, ptn.z+m_planes[i].normal().z);				
				glEnd(); 
			}
		}

		glColor4fv(savedColour);
		glDisable(GL_BLEND);
	}

	drawSceneBounds();
}
//-----------------------------------------------------------------------------
void ClippingTool::drawSceneBounds()
//-----------------------------------------------------------------------------
{	
	int numScenes = ptNumScenes();		
	if (numScenes)
	{
		PThandle *sceneHandles = new PThandle[numScenes];
		if (ptGetSceneHandles(sceneHandles))
		{
			double lower[3], upper[3];
			if (ptSceneBoundsd(sceneHandles[0], lower, upper) == PTV_SUCCESS)
			{
				GLfloat savedColour[4] = {0.0f};
				glGetFloatv(GL_CURRENT_COLOR, savedColour);

				glBegin(GL_LINES); 		

				glVertex3f(lower[0], lower[1], lower[2]);
				glVertex3f(upper[0], lower[1], lower[2]);

				glVertex3f(lower[0], lower[1], lower[2]);
				glVertex3f(lower[0], upper[1], lower[2]);

				glVertex3f(lower[0], lower[1], lower[2]);
				glVertex3f(lower[0], lower[1], upper[2]);

				glVertex3f(upper[0], upper[1], upper[2]);
				glVertex3f(lower[0], upper[1], upper[2]);

				glVertex3f(upper[0], upper[1], upper[2]);
				glVertex3f(upper[0], lower[1], upper[2]);

				glVertex3f(upper[0], upper[1], upper[2]);
				glVertex3f(upper[0], upper[1], lower[2]);

				glVertex3f(lower[0], upper[1], lower[2]);
				glVertex3f(upper[0], upper[1], lower[2]);

				glVertex3f(upper[0], lower[1], upper[2]);
				glVertex3f(lower[0], lower[1], upper[2]);

				glVertex3f(upper[0], lower[1], lower[2]);
				glVertex3f(upper[0], upper[1], lower[2]);

				glVertex3f(lower[0], upper[1], upper[2]);
				glVertex3f(lower[0], lower[1], upper[2]);

				glVertex3f(upper[0], lower[1], lower[2]);
				glVertex3f(upper[0], lower[1], upper[2]);

				glVertex3f(lower[0], upper[1], upper[2]);
				glVertex3f(lower[0], upper[1], lower[2]);

				glEnd(); 
				
				glColor4fv(savedColour);
			}
		}
	}
}
//-----------------------------------------------------------------------------
void ClippingTool::buildUserInterface(GLUI_Node *parent)
//-----------------------------------------------------------------------------
{
	GLUI_Rollout *rolloutSelect = new GLUI_Rollout(parent, "Clipping Planes", true );
	rolloutSelect->set_w( PANEL_WIDTH );

	GLUI_Panel *clipTools = new GLUI_Panel( rolloutSelect, " ", GLUI_PANEL_NONE);
	GLUI_StaticText *spacer = new GLUI_StaticText( clipTools, " " );
	spacer->set_w( PANEL_WIDTH*0.5 - 12 );

	m_fitPlanesButton = new GLUI_Button( clipTools, "Fit Planes to Bounds", CmdFitPlanesToBB, &Tool::dispatchCmd );			
	m_toggleClipping = new GLUI_Button( clipTools, "Switch Clipping Off", CmdToggleClipping, &Tool::dispatchCmd );			
	m_clipStyleButton = new GLUI_Button( clipTools, "Toggle Clip Style", CmdToggleClipStyle, &Tool::dispatchCmd );		
	spacer = new GLUI_StaticText( clipTools, "" );
	spacer->set_w( PANEL_WIDTH/2 );
	updateToggleClippingButtonText();
	
	m_planeDefList = new GLUI_List( clipTools, false, CmdSelectPlaneDef, &Tool::dispatchCmd );	
	m_planeDefList->set_w(PANEL_WIDTH);
	m_planeDefList->set_h(100);
	spacer = new GLUI_StaticText( clipTools, "" );
	spacer->set_w( PANEL_WIDTH );	
	updateListFromPlanes(false);
	
	m_planeDefInputVals[0] = 1.0f;
	m_planeDefInputs[0] = new GLUI_Spinner( clipTools, "x", &m_planeDefInputVals[0]);
	m_planeDefInputs[0]->set_w(100);
	m_planeDefInputs[0]->set_h(20);	
	
	m_planeDefInputVals[1] = 1.0f;
	m_planeDefInputs[1] = new GLUI_Spinner( clipTools, "y", &m_planeDefInputVals[1]);
	m_planeDefInputs[1]->set_w(100);
	m_planeDefInputs[1]->set_h(20);

	m_planeDefInputVals[2] = 1.0f;
	m_planeDefInputs[2] = new GLUI_Spinner( clipTools, "z", &m_planeDefInputVals[2]);
	m_planeDefInputs[2]->set_w(100);
	m_planeDefInputs[2]->set_h(20);	

	m_planeDefInputVals[3] = 1.0f;
	m_planeDefInputs[3] = new GLUI_Spinner( clipTools, "k", &m_planeDefInputVals[3]);
	m_planeDefInputs[3]->set_w(100);
	m_planeDefInputs[3]->set_h(20);

	m_togglePlaneButton  = new GLUI_Button( clipTools, "On/Off", CmdTogglePlane, &Tool::dispatchCmd );		
	spacer->set_w( PANEL_WIDTH/2 );

	m_updatePlaneButton = new GLUI_Button( clipTools, "Update", CmdUpdatePlane, &Tool::dispatchCmd );		
	spacer->set_w( PANEL_WIDTH/2 );

	// get the Vortex plane handles 
	int numPlanes = ptGetNumClippingPlanes();
	assert(numPlanes == CLIPPING_TOOL_NUM_PLANES); // this example assumes there are CLIPPING_TOOL_NUM_PLANES planes available in Vortex	

	// enable global clipping
	enableClipping(m_clippingOn);
}
//-----------------------------------------------------------------------------
void ClippingTool::fitPlanesToBB()
//-----------------------------------------------------------------------------
{
	int numScenes = ptNumScenes();		
	if (numScenes)
	{
		PThandle *sceneHandles = new PThandle[numScenes];
		if (ptGetSceneHandles(sceneHandles))
		{
			double lower[3], upper[3];
			if (ptSceneBoundsd(sceneHandles[0], lower, upper) == PTV_SUCCESS)
			{
				// set up 6 planes matching the bounds of the scene
				vortex::Vector3d pt0(lower[0], lower[1], lower[2]);
				vortex::Vector3d pt1(upper[0], lower[1], lower[2]);
				vortex::Vector3d pt2(lower[0], upper[1], lower[2]);
				vortex::Vector3d pt3(upper[0], upper[1], lower[2]);
															  
				vortex::Vector3d pt4(lower[0], lower[1], upper[2]);
				vortex::Vector3d pt5(upper[0], lower[1], upper[2]);
				vortex::Vector3d pt6(lower[0], upper[1], upper[2]);
				vortex::Vector3d pt7(upper[0], upper[1], upper[2]);
													
				planeFromPoints(0, pt0, pt2, pt1);
				planeFromPoints(1, pt1, pt3, pt5);
				planeFromPoints(2, pt5, pt7, pt4);
				planeFromPoints(3, pt4, pt6, pt0);
				planeFromPoints(4, pt2, pt6, pt3);
				planeFromPoints(5, pt0, pt1, pt4);

				updateListFromPlanes(true);
			}
		}
	}
}
//-----------------------------------------------------------------------------
// set the plane normal and constant for the plane with the passed planeIndex
// from the vectors pt1->pt2 and pt1->pt3
//-----------------------------------------------------------------------------
void ClippingTool::planeFromPoints(int planeIndex, vortex::Vector3d& pt1, vortex::Vector3d& pt2, vortex::Vector3d& pt3)
//-----------------------------------------------------------------------------
{
	if ((planeIndex < 0) || (planeIndex > CLIPPING_TOOL_NUM_PLANES-1))
		return;
	
	m_planes[planeIndex].from3points(pt1, pt2, pt3);	
}
//-----------------------------------------------------------------------------
void ClippingTool::updatePlane(unsigned int planeID)
//-----------------------------------------------------------------------------
{
	if ((planeID < 0) || (planeID > CLIPPING_TOOL_NUM_PLANES-1))
		return;

	// update the currently selected plane with the values in the A,B,C,D text inputs
	double a = 0.0, b = 0.0, c = 0.0, d = 0.0;
	const char* txt;

	if (txt = m_planeDefInputs[0]->get_text())
		a = atof(txt);
	if (txt = m_planeDefInputs[1]->get_text())
		b = atof(txt);
	if (txt = m_planeDefInputs[2]->get_text())
		c = atof(txt);
	if (txt = m_planeDefInputs[3]->get_text())
		d = atof(txt);
	
	vortex::Vector3d v(a, b, c);
	m_planes[planeID].update(v, d);	

	updateListFromPlanes(true, planeID);	
}
//-----------------------------------------------------------------------------
void ClippingTool::planeDefSelected(int selected)
//-----------------------------------------------------------------------------
{
	if ((selected < 0) || (selected > CLIPPING_TOOL_NUM_PLANES-1))
		return;

	// fill the A,B,C,D text boxes with the values in the selected plane def
	if (GLUI_List_Item* item = m_planeDefList->get_item_ptr(selected))
	{
		double a, b, c, d;

		if (const char* def = item->text.c_str())
		{
			getConstValsFromDef(def, a, b, c, d);
		
			m_planeDefInputs[0]->set_float_val(a);
			m_planeDefInputs[1]->set_float_val(b);		
			m_planeDefInputs[2]->set_float_val(c);
			m_planeDefInputs[3]->set_float_val(d);
		}
	}
}
//-----------------------------------------------------------------------------
void ClippingTool::toggleSelectedPlane(int selected)
//-----------------------------------------------------------------------------
{
	if ((selected < 0) || (selected > CLIPPING_TOOL_NUM_PLANES-1))
		return;

	bool enabled = ptIsClippingPlaneEnabled(selected);
	if (!enabled) 
		ptEnableClippingPlane(selected);
	else
		ptDisableClippingPlane(selected);
}
//-----------------------------------------------------------------------------
void ClippingTool::getConstValsFromDef(const char* def, double& a, double& b, double& c, double& d)
//-----------------------------------------------------------------------------
{
	// string def is in the format:
	// Plane 0: Ax + By + Cz = D
	a = getConst(def, "x");
	b = getConst(def, "y");
	c = getConst(def, "z");
	d = getConst(def, NULL);
}
//-----------------------------------------------------------------------------
double ClippingTool::getConst(const char* def, const char* constLetter)
//-----------------------------------------------------------------------------
{
	// string def is in the format:
	// Plane 0: Ax + By + Cz = D
	double ret = 0;
	
	std::string str(def);
	size_t endPos = strlen(def)-1;
	if (constLetter) // constLetter is x, y, z or NULL to find the last const in the string
		endPos= str.find(constLetter)-1;
	size_t startPos = 0;
	if (endPos > 0)
	{
		for (size_t i = endPos; i >= 0; i--)
		{
			if (def[i] == ' ')
			{
				startPos = i+1;
				break;
			}
		}
	}
	char num[64] = {0};
	strncpy(num, &def[startPos], endPos-startPos+1);
	ret = atof(num); 

	return ret;
}
//-----------------------------------------------------------------------------
void ClippingTool::updateListFromPlanes(bool enablePlanes, int keepSelected)
//-----------------------------------------------------------------------------
{
	if (m_planeDefList)
	{
	//	m_planeDefList->delete_all();

		for (int i = 0; i < CLIPPING_TOOL_NUM_PLANES; i++)
		{
			char txt[64] = {0};	
			sprintf(txt, "Plane %d: %.2fx + %.2fy + %.2fz = %.2f", i, m_planes[i].normal().x, m_planes[i].normal().y, m_planes[i].normal().z, m_planes[i].constant());

			if (GLUI_List_Item* item = m_planeDefList->get_item_ptr(i))	
				item->text = txt;			
			else
				m_planeDefList->add_item(i, txt);	

			if (enablePlanes)
			{
				// update this plane's parameters in Vortex				
				ptSetClippingPlaneParameters(i, m_planes[i].normal().x, m_planes[i].normal().y, m_planes[i].normal().z, m_planes[i].constant());
				ptEnableClippingPlane(i);
			}
		}

		if (keepSelected != -1)
		{
			// TODO: maintain the previous selection after the plane list is refilled
		}		
	}
}

void ClippingTool::enableClipping(bool enable)
{
	m_clippingOn = enable;
	if (m_clippingOn)
		ptEnableClipping();
	else
		ptDisableClipping();
}

void ClippingTool::updateToggleClippingButtonText()
{
	if (m_toggleClipping)
	{
		m_toggleClipping->set_name((m_clippingOn) ? "Switch Clipping Off" : "Switch Clipping On");
	}
}