/******************************************************************************

Pointools Vortex API Examples

ClippingTool.h

Demonstrates layer based point editing capabilities of Vortex

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_CLIPPING_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_CLIPPING_TOOL_H_

#include "VortexExampleApp.h"
#include <PointoolsVortexAPI_FeatureExtract_DLL/GeomTypes.h>
#include "Plane.h"
#include <assert.h>


#define CLIPPING_TOOL_NUM_PLANES 6

class ClippingTool : public Tool
{
public:
	enum
	{
		CmdFitPlanesToBB = 1300,
		CmdToggleClipStyle,
		CmdSelectPlaneDef,
		CmdUpdatePlane,		
		CmdTogglePlane,		
		CmdToggleClipping
	};

	ClippingTool(bool simple);	// simple = true, for no layers or scope 
	
	void	buildUserInterface(GLUI_Node *parent);
	void	command( int cmdId );

	bool	onMouseButtonDown( int button, int x, int y );
	bool	onMouseButtonUp( int button, int x, int y );
	bool	onMouseMove( int x, int y );
	bool	onMouseDrag( int x, int y, int startX, int startY );

	void	drawPostDisplay();	

private:
	void	drawSceneBounds();

	void	fitPlanesToBB();
	void	planeFromPoints(int planeIndex, vortex::Vector3d& pt1, vortex::Vector3d& pt2, vortex::Vector3d& pt3);
	void	updatePlane(unsigned int planeID);
	void	planeDefSelected(int selected);
	void	toggleSelectedPlane(int selected);
	void	getConstValsFromDef(const char* def, double& a, double& b, double& c, double& d);
	double	getConst(const char* def, const char* constLetter);
	void	updateListFromPlanes(bool enablePlanes, int keepSelected = -1);
	void	enableClipping(bool enable);
	void	updateToggleClippingButtonText();

	Mouse				m_mouse;
	std::vector<int>	m_polygon;
	bool				m_clippingOn;

	// Button for fitting the clipping planes to the bounding box of the currently loaded cloud
	GLUI_Button*		m_fitPlanesButton;

	GLUI_Button*		m_clipStyleButton;
	int					m_clipStyle;

	// float input and buttons for getting the plane equations for each plane Ax + By + Cz = D
	GLUI_Spinner*		m_planeDefInputs[4];
	float				m_planeDefInputVals[4];	

	GLUI_List*			m_planeDefList;
	GLUI_Button*		m_togglePlaneButton;
	GLUI_Button*		m_updatePlaneButton;
	GLUI_Button*		m_toggleClipping;

	vortex::Planed		m_planes[CLIPPING_TOOL_NUM_PLANES];	
};

#endif

