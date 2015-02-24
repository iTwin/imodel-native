/******************************************************************************

Pointools Vortex API Examples

SnapTest.h

Demonstrates methods for implementing point snaps

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_SNAP_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_SNAP_TOOL_H_

#include "VortexExampleApp.h"
#include <string>

class SnapTest : public Tool
{
public:
	enum
	{
		CmdMeasurePnt = 700
	};

	SnapTest();

	void			buildUserInterface(GLUI_Node *parent);
	void			command( int cmdId );
	
	const double	*lastSnapPoint() const;	// returns null for invalid point

	bool			onMouseButtonDown( int button, int x, int y );
	void			drawPostDisplay();

private:

	enum
	{
		PointInvalid		 = 0,
		PointApproximate	 = 1,
		PointNearest		 = 2,
		PointRayIntersection = 3
	};

	bool			findSnapPoint(int mx, int my);

	bool			m_active;
	std::string		m_outputString;
	GLUI_TextBox	*m_output;
	double			m_lastSnapPoint[3];
	
	int				m_lastPointType;
};

#endif

