/******************************************************************************

Pointools Vortex API Examples

SnapTest.cpp

Demonstrates methods for implementing point snaps

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#include "SnapTest.h"

//-----------------------------------------------------------------------------
SnapTest::SnapTest() : Tool(CmdMeasurePnt, CmdMeasurePnt)
//-----------------------------------------------------------------------------
{
	m_output = 0;
	m_active = false;
	m_lastPointType = PointInvalid;
}
//-----------------------------------------------------------------------------
void SnapTest::buildUserInterface( GLUI_Node *parent )
//-----------------------------------------------------------------------------
{
	GLUI_Rollout * rolloutTests = new GLUI_Rollout( parent, "Snapping", true );
	rolloutTests->set_w( PANEL_WIDTH );

	GLUI_StaticText *spacer = new GLUI_StaticText( rolloutTests, "" );
	spacer->set_w( PANEL_WIDTH );

	new GLUI_Button( rolloutTests, "Measure Pnt", CmdMeasurePnt, &Tool::dispatchCmd );

	spacer = new GLUI_StaticText( rolloutTests, "" );
	spacer->set_w( PANEL_WIDTH );

	/* stats output */ 
	m_outputString = "Output:\n-------------------";
	m_output = new GLUI_TextBox( rolloutTests, m_outputString, false );
	m_output->set_w( PANEL_WIDTH );
	m_output->set_h( 160 );
	
}
//-----------------------------------------------------------------------------
void SnapTest::command( int cmdId )
//-----------------------------------------------------------------------------
{
	if (cmdId == CmdMeasurePnt)
		m_active = true;
}
//-----------------------------------------------------------------------------
const double * SnapTest::lastSnapPoint() const
//-----------------------------------------------------------------------------
{
	if (m_lastPointType)
		return m_lastSnapPoint;
	else return 0;
}
//-----------------------------------------------------------------------------
bool SnapTest::onMouseButtonDown( int button, int x, int y )
//-----------------------------------------------------------------------------
{
	if (m_active)
	{
		switch (button)
		{
		case MouseLeftButton:
			findSnapPoint(x,y);
			Tool::viewUpdate();
			return true;
			break;

		case MouseRightButton:
			m_active = false;
			return true;		
			break;
		}
	}
	return false;
}
//-----------------------------------------------------------------------------
bool SnapTest::findSnapPoint( int mx, int my )
//-----------------------------------------------------------------------------
{
	pt::PerformanceTimer t0;
	t0.start();

	PTdouble pnt[3];

	// find nearest screen point based on reading GL buffer and searching for point
	int res = ptFindNearestScreenPoint( 0, mx, my, pnt );

	if (res >=0 )	
	{
		/* this point maybe good enough, but a ray intersection should give us something
		that is closer to the viewer and potentially a better point */ 

		/* generate a ray from this using the camera position*/ 
		// compute the cam position from the modelview matrix
		GLdouble mdl[16];    
		GLdouble camera[3];    
		glGetDoublev(GL_MODELVIEW_MATRIX, mdl);    
		camera[0] = -(mdl[0] * mdl[12] + mdl[1] * mdl[13] + mdl[2] * mdl[14]);
		camera[1] = -(mdl[4] * mdl[12] + mdl[5] * mdl[13] + mdl[6] * mdl[14]);
		camera[2] = -(mdl[8] * mdl[12] + mdl[9] * mdl[13] + mdl[10] * mdl[14]);

		double dir[] = { pnt[0] - camera[0], pnt[1] - camera[1], pnt[2] - camera[2] };

		ptSetIntersectionRadius( 0.005f );
		
		/* do intersection test */ 
		bool didIntersect = ptIntersectRay(0, camera, dir, pnt, PT_QUERY_DENSITY_VIEW, 1);

		// get timing information
		t0.end();
		double millis = t0.millisecs();
		
		char output[256];

		if (didIntersect)
		{			
			sprintf( output, "\nIntersection at %0.3f, %0.3f, %0.3f (%dms)",
				pnt[0], pnt[1], pnt[2], (int)millis );

			m_lastPointType = PointRayIntersection;
		}
		else
		{	
			sprintf( output, "\nNearest point is %0.3f, %0.3f, %0.3f (%dms)",
				pnt[0], pnt[1], pnt[2], (int)millis );

			m_lastPointType = PointNearest;
		}
		m_outputString = output;	

		m_lastSnapPoint[0] = pnt[0];
		m_lastSnapPoint[1] = pnt[1];
		m_lastSnapPoint[2] = pnt[2];
	}
	if (res < 0) //no real intersection 
	{
		m_outputString += "\nReal point not found, approximation used";	// ie GL unproject position

		m_lastPointType = PointApproximate;
	}
	m_output->set_text( m_outputString.c_str() );
	m_output->redraw();

	return true;
}
//-----------------------------------------------------------------------------
void SnapTest::drawPostDisplay()
//-----------------------------------------------------------------------------
{
	if (m_lastPointType)
	{
		double scale = 1.0;
		
		switch (m_lastPointType)
		{
		case PointApproximate:
			glColor3f(1.0f,0,0);
			break;

		case PointNearest:
			glColor3f(0,1.0f,0);
			break;

		case PointRayIntersection:
			glColor3f(0,1.0f,1.0f);
			break;
		}
		glBegin( GL_LINES );
			glVertex3d( m_lastSnapPoint[0]+scale, m_lastSnapPoint[1], m_lastSnapPoint[2] );
			glVertex3d( m_lastSnapPoint[0]-scale, m_lastSnapPoint[1], m_lastSnapPoint[2] );

			glVertex3d( m_lastSnapPoint[0], m_lastSnapPoint[1]+scale, m_lastSnapPoint[2] );
			glVertex3d( m_lastSnapPoint[0], m_lastSnapPoint[1]-scale, m_lastSnapPoint[2] );

			glVertex3d( m_lastSnapPoint[0], m_lastSnapPoint[1], m_lastSnapPoint[2]+scale );
			glVertex3d( m_lastSnapPoint[0], m_lastSnapPoint[1], m_lastSnapPoint[2]-scale );
		glEnd();
	}
}


